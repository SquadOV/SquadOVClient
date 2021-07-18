#pragma once

#include "recorder/pipe/cloud/cloud_storage_client.h"
#include "shared/http/http_client.h"

namespace service::recorder::pipe::cloud {

class S3StorageClient: public CloudStorageClient {
public:
    explicit S3StorageClient(const std::string& videoUuid);

    void initializeDestination(const service::vod::VodDestination& destination) override;
    void setMaxUploadSpeed(size_t bytesPerSec) override;

    void startNewSegment() override;
    std::pair<std::string, size_t> uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) override;
private:
    std::string _videoUuid;
    service::vod::VodDestination _destination;
    shared::http::HttpClientPtr _httpClient;
    size_t _currentPart = 1;
};

}