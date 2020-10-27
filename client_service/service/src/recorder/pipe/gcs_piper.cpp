#include "recorder/pipe/gcs_piper.h"

#include "shared/errors/error.h"
#include "shared/filesystem/common_paths.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <sstream>

namespace service::recorder::pipe {

constexpr size_t GCS_BUFFER_SIZE_BYTES = 8 * 1024 * 1024;
constexpr size_t GCS_CHUNK_SIZE_MULTIPLE = 256 * 1024;
constexpr int GCS_MAX_RETRIES = 10;
constexpr int GCS_MAX_BACKOFF_TIME_MS = 32000;

#define LOG_TIME 0

GCSPiper::GCSPiper(const std::string& destination, PipePtr&& pipe):
    FileOutputPiper(std::move(pipe)),
    _gen(_rd()) {

    // Create a common HTTP client to use for the upload requests.
    // I don't think we technically need to use the *same* client but
    // no reason not to save ourselves from reallocating it.
    _httpClient = std::make_unique<service::http::HttpClient>("");

    // The signed URL should return a header with the "Location" key.
    // This is the URL we'll use for all future upload requests.
    _httpClient->addResponseInterceptor([this](service::http::HttpResponse& response){
        for (const auto& [key, value] : response.headers) {
            if (key == "Location") {
                _sessionUri = value;
                break;
            }
        }
        return true;
    });
    _httpClient->setHeaderKeyValue("x-goog-resumable", "start");
    _httpClient->setHeaderKeyValue("content-type", "application/octet-stream");
    const auto ret = _httpClient->post(destination, nlohmann::json());
    if (ret->status != 201) { 
        THROW_ERROR("Failed to POST to GCS signed URL: " << ret->status << "\t" << ret->body << std::endl);
    }
    _httpClient->removeHeaderKey("x-goog-resumable");
    _httpClient->clearResponseInterceptors();
    _gcsThread = std::thread(std::bind(&GCSPiper::tickGcsThread, this));

#if DUMP_GCS_REF_VIDEO
    const auto path = shared::filesystem::getSquadOvUserFolder() / std::filesystem::path("gcs_ref.mp4");
    _refVideo.open(path.string(), std::ios_base::binary | std::ios_base::out);
#endif
}

GCSPiper::~GCSPiper() {
    flush();
}

void GCSPiper::copyDataIntoInternalBuffer(std::vector<char>& buffer) {
    std::unique_lock<std::mutex> guard(_gcsMutex);
    if (_gcsBuffer.size() >= 0) {
        for (const auto& packet : _gcsBuffer) {
            const auto oldSize = buffer.size();

            // Ensure that the internal buffer has enough space to handle the next packet
            // before memcpying into this new space. Ideally this should be fairly cheap
            // as we won't go over the pre-reserved buffer size too often.
            buffer.resize(buffer.size() + packet.size());
            std::memcpy(buffer.data() + oldSize, packet.data(), packet.size());
        }
    }

    _gcsBuffer.clear();
}

void GCSPiper::tickGcsThread() {
    std::vector<char> internalBuffer;
    internalBuffer.reserve(GCS_BUFFER_SIZE_BYTES);

    while (!_finished) {
#if LOG_TIME
        const auto taskNow = std::chrono::high_resolution_clock::now();
        const auto oldBufferSize = internalBuffer.size();
#endif
        // If the write buffer (_gcsBuffer) is non empty, copy the data
        // into the internalBuffer. The reason we do this is to ensure that
        // handleBuffer can keep running and pulling data from wherever it's
        // getting data from without having to wait on the GCS request to finish
        // (even when we have to do backoff to retry a request).
        copyDataIntoInternalBuffer(internalBuffer);

#if LOG_TIME
        const auto bufferSize = internalBuffer.size();
        {
            const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
            const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
            LOG_INFO("GCS Copy Buffer: " << numMs << "ms [" << (bufferSize - oldBufferSize) << " bytes]" << std::endl);
        }
#endif

        if (internalBuffer.size() >= GCS_BUFFER_SIZE_BYTES) {
            sendDataFromBufferToGcsWithBackoff(internalBuffer, GCS_BUFFER_SIZE_BYTES, false);
        }

#if LOG_TIME
        {
            const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
            const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
            LOG_INFO("GCS Copy+Send Buffer: " << numMs << "ms [" << bufferSize << " bytes]" << std::endl);
        }
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Need to do one last upload to let GCS know that we're finished
    // by indicating the size of the content in the header range.
    copyDataIntoInternalBuffer(internalBuffer);

    bool first = true;
    while (first || !internalBuffer.empty()) {
        sendDataFromBufferToGcsWithBackoff(internalBuffer, 0, true);
        first = false;
    }
}

// This function handles two things
//  1) If the internal request to GCS fails (sendDataFromBufferToGcs), then it'll do truncated
//     exponential backoff to keep retrying the upload for some maximum number of runs.
//  2) It'll remove the bytes that we successfully sent from the buffer. There's no guarantee
//     that we successfully upload all the bytes to GCS so we make sure to keep only the bytes
//     we successfully sent.
void GCSPiper::sendDataFromBufferToGcsWithBackoff(std::vector<char>& buffer, size_t maxBytes, bool isLast) {
    static std::uniform_int_distribution<> backoffDist(0 , 1000);
    const auto requestedBytes = (maxBytes == 0) ? buffer.size() : std::min(maxBytes, buffer.size());
    for (auto i = 0; i < GCS_MAX_RETRIES; ++i) {
        try {
            const auto bytesSent = sendDataFromBufferToGcs(buffer.data(), requestedBytes, isLast);
            if (bytesSent == buffer.size()) {
                buffer.clear();
            } else {
                buffer.erase(buffer.begin(), buffer.begin() + bytesSent);
            }
            _uploadedBytes += bytesSent;
            return;
        } catch(std::exception& ex) {
            LOG_WARNING("GCS Upload Failure: " << ex.what() << std::endl);

            const auto backoffMs = std::min(static_cast<int>(std::pow(2, i)) +  backoffDist(_gen), GCS_MAX_BACKOFF_TIME_MS);
            std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
        }
    }

    // If we get here we know that we encountered some sort of error with GCS.
    // Scrap this and stop sending data. TODO.
}

size_t GCSPiper::sendDataFromBufferToGcs(const char* buffer, size_t numBytes, bool isLast) {
    _httpClient->setHeaderKeyValue("Content-Length", std::to_string(numBytes));

    std::ostringstream range;
    range << "bytes " << _uploadedBytes << "-" << (_uploadedBytes + numBytes - 1) << "/";
    if (isLast) {
        range << (_uploadedBytes + numBytes);
    } else {
        range << "*";
    }
    _httpClient->setHeaderKeyValue("Content-Range", range.str());

    std::string retRange;
    _httpClient->addResponseInterceptor([&retRange](service::http::HttpResponse& response){
        for (const auto& [key, value] : response.headers) {
            if (key == "Range") {
                retRange = value.substr(value.find("bytes=") + 6);
                break;
            }
        }
        return true;
    });

    const auto ret = _httpClient->put(_sessionUri, buffer, numBytes);
    if (ret->status == 200 || ret->status == 201) {
        // 200/201 indicate that the upload was completed.
        return numBytes;
    } else if (ret->status == 308) {
        // 308 indicates that we need to continue uploading - we should use the Range header (retRange)
        // to determine how many bytes were actually sent. If no range header was returned then no
        // data has been uploaded.
        if (retRange.empty()) {
            return 0;
        }

        std::vector<std::string> rangeParts;
        boost::split(rangeParts, retRange, boost::is_any_of("-"));
        if (rangeParts.size() == 2) {
            // Use our own start here because GCS will return 0-X
            // to indicate *all* the data we've uploaded so far not just
            // this request.
            const auto start = _uploadedBytes;
            const auto end = std::stoi(rangeParts[1]);
            return (end - start + 1);
        } else {
            THROW_ERROR("Invalid range header received: " << retRange << std::endl);
        }

    } else {
        THROW_ERROR("Failed to send data to GCS: " << ret->status << std::endl);
    }

    return 0;
}

bool GCSPiper::handleBuffer(const char* buffer, size_t numBytes) {
    if (numBytes == 0) {
        // At this point we know that we're done receiving data and all that's left is to
        // flush out GCS.
        return true;
    }

#if LOG_TIME
    const auto taskNow = std::chrono::high_resolution_clock::now();
#endif
    GCSPacket packet(buffer, numBytes);

    std::unique_lock<std::mutex> guard(_gcsMutex);
    _gcsBuffer.emplace_back(std::move(packet));

#if DUMP_GCS_REF_VIDEO
    _refVideo.write(packet.data(), packet.size());
#endif

#if LOG_TIME
    const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
    const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
    LOG_INFO("GCS Handle Buffer: " << numMs << "ms" << std::endl);
#endif

    return true;
}

void GCSPiper::flush() {
    if (_finished) {
        return;
    }

    _finished = true;

    if (_gcsThread.joinable()) {
        _gcsThread.join();
    }

#if DUMP_GCS_REF_VIDEO
    _refVideo.close();
#endif
}

}