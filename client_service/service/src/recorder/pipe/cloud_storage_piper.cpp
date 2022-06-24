#include "recorder/pipe/cloud_storage_piper.h"

#include "shared/errors/error.h"
#include "shared/filesystem/common_paths.h"
#include "recorder/pipe/cloud/gcs_storage_client.h"
#include "recorder/pipe/cloud/s3_storage_client.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
namespace fs = std::filesystem;
namespace service::recorder::pipe {

constexpr size_t CLOUD_BUFFER_SIZE_BYTES = 8 * 1024 * 1024;
constexpr size_t CLOUD_CHUNK_SIZE_MULTIPLE = 256 * 1024;
constexpr int CLOUD_MAX_RETRIES = 10;
constexpr int CLOUD_MAX_BACKOFF_TIME_MS = 96000;

#define LOG_TIME 0

CloudUploadRequest CloudUploadRequest::fromJson(const nlohmann::json& json) {
    CloudUploadRequest ret;
    ret.file = json["file"].get<std::string>();
    ret.task = json["task"].get<std::string>();
    ret.destination = service::uploader::UploadDestination::fromJson(json["destination"]);
    ret.checkMetadata = json.value("checkMetadata", false);
    return ret;
}

std::pair<std::string, std::vector<std::string>> uploadToCloud(const CloudUploadRequest& req, const shared::http::DownloadUploadProgressFn& progressFn) {
    // It's a bit of a roundabout way to do it but create a local system pipe to write the file data to
    // and then just wait for the pipe to finish.
    auto pipe = std::make_unique<Pipe>(req.task);
    const auto pipePath = pipe->filePath();
    std::string inputFname = req.file;
    
    const std::string filePrefix = "file:///";
    const auto it = inputFname.find(filePrefix);
    if (it != std::string::npos) {
        inputFname.replace(it, it + filePrefix.size(), "");
    }

    const auto inputPath = fs::path(inputFname);
    CloudStoragePiper output(req.task, req.destination, std::move(pipe), true);
    output.setProgressCallback(progressFn, fs::file_size(inputPath));
    output.appendFromFile(inputPath);
    output.sendNullBuffer();
    output.flush();
    return std::make_pair(output.sessionId(), output.segmentIds());
}

CloudStoragePiper::CloudStoragePiper(const std::string& videoUuid, const service::uploader::UploadDestination& destination, PipePtr&& pipe, bool allowAcceleration):
    FileOutputPiper(std::move(pipe)),
    _videoUuid(videoUuid),
    _destination(destination),
    _gen(_rd()) {

    // Create an appropriate storage client based on the input destination location.
    switch (destination.loc) {
        case service::uploader::UploadManagerType::S3:
            _client = std::make_unique<cloud::S3StorageClient>(_videoUuid, allowAcceleration);
            break;
        case service::uploader::UploadManagerType::GCS:
            _client = std::make_unique<cloud::GCSStorageClient>();
            break;
    }

    if (!_client) {
        THROW_ERROR("Invalid cloud storage piper location [or failed to alloc]: " << destination);
        return;
    }

    _timeStart = shared::nowUtc();
    _client->initializeDestination(destination);
    _cloudThread = std::thread(std::bind(&CloudStoragePiper::tickUploadThread, this));

#if DUMP_CLOUD_REF_VIDEO
    const auto path = shared::filesystem::getSquadOvUserFolder() / std::filesystem::path("CLOUD_ref.ts");
    _refVideo.open(path, std::ios_base::binary | std::ios_base::out);
#endif
}

CloudStoragePiper::~CloudStoragePiper() {
    flush();
}

void CloudStoragePiper::setProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn, size_t totalBytes) {
    _progressFn = progressFn;
    _totalUploadBytes = totalBytes;
}

void CloudStoragePiper::setCurlProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn) {
    _client->setProgressCallback(progressFn);
}

void CloudStoragePiper::copyDataIntoInternalBuffer(std::vector<char>& buffer) {
    std::unique_lock<std::mutex> guard(_cloudMutex);
    if (_cloudBuffer.size() >= 0) {
        for (const auto& packet : _cloudBuffer) {
            const auto oldSize = buffer.size();

            // Ensure that the internal buffer has enough space to handle the next packet
            // before memcpying into this new space. Ideally this should be fairly cheap
            // as we won't go over the pre-reserved buffer size too often.
            buffer.resize(buffer.size() + packet.size());
            std::memcpy(buffer.data() + oldSize, packet.data(), packet.size());
        }
    }

    _cloudBuffer.clear();
}

void CloudStoragePiper::tickUploadThread() {
    std::vector<char> internalBuffer;
    internalBuffer.reserve(CLOUD_BUFFER_SIZE_BYTES);

    try {
        while (!_finished) { 
#if LOG_TIME
            const auto taskNow = std::chrono::high_resolution_clock::now();
            const auto oldBufferSize = internalBuffer.size();
#endif
            // If the write buffer (_cloudBuffer) is non empty, copy the data
            // into the internalBuffer. The reason we do this is to ensure that
            // handleBuffer can keep running and pulling data from wherever it's
            // getting data from without having to wait on the upload HTTP request to finish
            // (even when we have to do backoff to retry a request).
            copyDataIntoInternalBuffer(internalBuffer);

#if LOG_TIME
            const auto bufferSize = internalBuffer.size();
            {
                const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
                const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
                LOG_INFO("Cloud Copy Buffer: " << numMs << "ms [" << (bufferSize - oldBufferSize) << " bytes]" << std::endl);
            }
#endif

            if (internalBuffer.size() >= CLOUD_BUFFER_SIZE_BYTES) {
                sendDataFromBufferWithBackoff(internalBuffer, CLOUD_BUFFER_SIZE_BYTES, false);
            }

#if LOG_TIME
            {
                const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
                const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
                LOG_INFO("Cloud Copy+Send Buffer: " << numMs << "ms [" << bufferSize << " bytes]" << std::endl);
            }
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        // Need to do one last upload to let the cloud storage know that we're finished.
        copyDataIntoInternalBuffer(internalBuffer);

        bool first = true;
        while ((first || !internalBuffer.empty()) && !_skipFlush) {
            sendDataFromBufferWithBackoff(internalBuffer, 0, true);
            first = false;
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Failed to finish upload: " << ex.what() << std::endl);
        if (_throwOnError) {
            throw ex;
        }
    }
}

// This function handles two things
//  1) If the internal request to upload to cloud storage fails, then it'll do truncated
//     exponential backoff to keep retrying the upload for some maximum number of runs.
//  2) It'll remove the bytes that we successfully sent from the buffer. There's no guarantee
//     that we successfully upload all the bytes so we make sure to keep only the bytes
//     we successfully sent.
void CloudStoragePiper::sendDataFromBufferWithBackoff(std::vector<char>& buffer, size_t maxBytes, bool isLast) {
    static std::uniform_int_distribution<> backoffDist(0, 3000);
    const auto requestedBytes = (maxBytes == 0) ? buffer.size() : std::min(maxBytes, buffer.size());
    const int totalRetries = _maxRetries ? std::min(static_cast<int>(_maxRetries.value()), CLOUD_MAX_RETRIES) : CLOUD_MAX_RETRIES;
    for (auto i = 0; i < totalRetries; ++i) {
        try {
            const auto data = _client->uploadBytes(buffer.data(), requestedBytes, isLast, _uploadedBytes);
            const auto bytesSent = data.second;
            if (bytesSent >= buffer.size()) {
                buffer.clear();
            } else {
                buffer.erase(buffer.begin(), buffer.begin() + bytesSent);
            }
            _uploadedBytes += bytesSent;
            _lastUploadTime = shared::nowUtc();
            if (_totalUploadBytes.has_value() && _progressFn.has_value()) {
                _progressFn.value()(0, 0, _totalUploadBytes.value(), _uploadedBytes);
            }

            if (!data.first.empty()) {
                _allSegmentsIds.push_back(data.first);
            }

            // At this point we can consider that we finished uploading a segment. Get a new segment if needed.
            if (!isLast) {
                _client->startNewSegment();
            }

            return;
        } catch(std::exception& ex) {
            LOG_WARNING("Cloud Upload Failure: " << ex.what() << std::endl);

            const auto backoffMs = std::min(1000 + static_cast<int>(std::pow(2, i)) +  backoffDist(_gen), CLOUD_MAX_BACKOFF_TIME_MS);
            std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
        }
    }

    // If we get here we know that we encountered some sort of error with GCS.
    // Scrap this and stop sending data.
    THROW_ERROR("Failed to upload file to cloud storage.");
}

bool CloudStoragePiper::handleBuffer(const char* buffer, size_t numBytes) {
    if (numBytes == 0) {
        // At this point we know that we're done receiving data and all that's left is to
        // flush out to cloud storage.
        return true;
    }

#if LOG_TIME
    const auto taskNow = std::chrono::high_resolution_clock::now();
#endif
    CloudStoragePacket packet(buffer, numBytes);

    std::unique_lock<std::mutex> guard(_cloudMutex);
    _cloudBuffer.emplace_back(std::move(packet));

#if DUMP_CLOUD_REF_VIDEO
    _refVideo.write(packet.data(), packet.size());
#endif

#if LOG_TIME
    const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
    const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
    LOG_INFO("Cloud Handle Buffer: " << numMs << "ms" << std::endl);
#endif

    return true;
}

void CloudStoragePiper::flush() {
    if (_finished) {
        return;
    }

    _finished = true;

    if (_cloudThread.joinable()) {
        _cloudThread.join();
    }

#if DUMP_CLOUD_REF_VIDEO
    _refVideo.close();
#endif
}

void CloudStoragePiper::setMaxUploadSpeed(std::optional<size_t> bytesPerSec) {
    if (bytesPerSec) {
        _client->setMaxUploadSpeed(bytesPerSec.value());
    }
}

void CloudStoragePiper::setMaxRetries(size_t retries) {
    _maxRetries = retries;
    _client->setMaxRetries(retries);
}

void CloudStoragePiper::setMaxTimeout(size_t timeoutSeconds) {
    _client->setMaxTimeout(timeoutSeconds);
}

}