#pragma once

#include "recorder/pipe/cloud/cloud_storage_client.h"
#include "shared/http/http_client.h"

namespace service::recorder::pipe::cloud {

class S3StorageClient: public CloudStorageClient {
public:
    S3StorageClient(const std::string& videoUuid, bool allowAcceleration);

    void initializeDestination(const service::uploader::UploadDestination& destination) override;
    void setProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn) override;
    void startNewSegment() override;
    std::pair<std::string, size_t> uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) override;
    void setMaxUploadSpeed(size_t bytesPerSec) override;
private:
    std::string _videoUuid;
    service::uploader::UploadDestination _destination;
    shared::http::HttpClientPtr _httpClient;
    size_t _currentPart = 1;
    bool _needsTransferAcceleration = false;
};

}