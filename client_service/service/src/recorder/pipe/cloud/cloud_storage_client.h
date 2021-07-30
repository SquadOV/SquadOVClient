#pragma once

#include <memory>
#include <utility>

#include "vod/vod.h"

namespace service::recorder::pipe::cloud {

class CloudStorageClient {
public:
    virtual void initializeDestination(const service::vod::VodDestination& destination) = 0;

    virtual void startNewSegment() = 0;
    virtual std::pair<std::string, size_t> uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) = 0;
};

using CloudStorageClientPtr = std::unique_ptr<CloudStorageClient>;

}