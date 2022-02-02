#include "recorder/pipe/cloud/s3_storage_client.h"
#include "api/squadov_api.h"
#include "system/settings.h"

namespace service::recorder::pipe::cloud {
namespace {

const auto MAX_NEW_SEGMENT_TRIES = 5;

}

S3StorageClient::S3StorageClient(const std::string& videoUuid, bool allowAcceleration):
    _videoUuid(videoUuid) 
{
    _httpClient = std::make_unique<shared::http::HttpClient>("");
    _httpClient->setHeaderKeyValue("content-type", "application/octet-stream");

    if (allowAcceleration) {
        service::system::getCurrentSettings()->reloadSettingsFromFile();
        _needsTransferAcceleration = (service::system::getCurrentSettings()->speedCheckResultMbps() < 16.0);
    }
}

void S3StorageClient::initializeDestination(const service::uploader::UploadDestination& destination) {
    _destination = destination;
}

void S3StorageClient::setProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn) {
    _httpClient->addDownloadProgressFn(progressFn);
}

void S3StorageClient::startNewSegment() {
    // Get the next destination to upload to.
    ++_currentPart;

    for (auto i = 0; i < MAX_NEW_SEGMENT_TRIES; ++i) {
        try {
            _destination = service::api::getGlobalApi()->getObjectPartUploadUri(_videoUuid, _destination.bucket, _destination.session, _destination.purpose, _currentPart, _needsTransferAcceleration);
            return;
        } catch (std::exception& ex) {
            LOG_WARNING("...Failed to get new segment [retrying] " << ex.what() << std::endl);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }

    THROW_ERROR("Failed to start new AWS multipart upload segment.");
}

std::pair<std::string, size_t> S3StorageClient::uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) {
    if (numBytes == 0) {
        // This is possible due to the flexibility between AWS and GCloud.
        // It's necessary for GCloud to finish off the upload, breaks things in AWS
        // so we ignore this request.
        return std::make_pair("", 0);
    }

    // Dynamically set a timeout based on the number of bytes we want to send.
    // The timeout should be at least 30 seconds long to hopefully protect against any unexpected internet issues.
    // Otherwise, assume a worst case scenario of uploading at 100KB/s. We do this instead of setting
    // a static timeout when the http client is made because uploading a bigger chunk of data
    // inherently needs a longer time to upload; hence it's more reasonable to dynamically choose
    // a timeout based on how many bytes we want to send.
    _httpClient->setTimeout(std::max(static_cast<long>(numBytes / (100 * 1024)), 30L));

    // Need to upload the buffer into the current destination (AWS S3 part) using the signed URL
    // stored in the _destination variable. Afterwards, we return the number of bytes we uploaded
    // (should be everything) along with the ETag.
    const auto ret = _httpClient->put(_destination.url, buffer, numBytes);
    if (ret->status != 200) {
        THROW_ERROR("Failed to upload to AWS part: " << ret->status << "\t" << ret->body);
        return std::make_pair("", 0);
    } else {
        const auto it = ret->headers.find("ETag");
        if (it == ret->headers.end()) {
            THROW_ERROR("Failed to obtain AWS part E-Tag.");
            return std::make_pair("", 0);
        }
        return std::make_pair(it->second, numBytes);
    }
}

}