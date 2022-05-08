#pragma once

#include <memory>
#include <utility>

#include "uploader/uploader.h"
#include "shared/http/http_client.h"

namespace service::recorder::pipe::cloud {

class CloudStorageClient {
public:
    virtual void initializeDestination(const service::uploader::UploadDestination& destination) = 0;

    virtual void startNewSegment() = 0;
    virtual std::pair<std::string, size_t> uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) = 0;
    virtual void setProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn) = 0;
    virtual void setMaxUploadSpeed(size_t bytesPerSec) = 0;
    virtual void setMaxRetries(size_t retries) = 0;
    virtual void setMaxTimeout(size_t timeoutSeconds) = 0;
};

using CloudStorageClientPtr = std::unique_ptr<CloudStorageClient>;

}