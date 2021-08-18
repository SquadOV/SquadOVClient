#include "recorder/pipe/cloud/s3_storage_client.h"
#include "api/squadov_api.h"

namespace service::recorder::pipe::cloud {
namespace {

const auto MAX_NEW_SEGMENT_TRIES = 5;

}

S3StorageClient::S3StorageClient(const std::string& videoUuid):
    _videoUuid(videoUuid) 
{
    _httpClient = std::make_unique<shared::http::HttpClient>("");
    _httpClient->setHeaderKeyValue("content-type", "application/octet-stream");
}

void S3StorageClient::initializeDestination(const service::vod::VodDestination& destination) {
    _destination = destination;
}

void S3StorageClient::startNewSegment() {
    // Get the next destination to upload to.
    ++_currentPart;

    for (auto i = 0; i < MAX_NEW_SEGMENT_TRIES; ++i) {
        try {
            _destination = service::api::getGlobalApi()->getVodPartUploadUri(_videoUuid, _destination.bucket, _destination.session, _currentPart);
            return;
        } catch (std::exception& ex) {
            LOG_WARNING("...Failed to get new segment [retrying] " << ex.what() << std::endl);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }

    THROW_ERROR("Failed to start new AWS multipart upload segment.");
}

std::pair<std::string, size_t> S3StorageClient::uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) {
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