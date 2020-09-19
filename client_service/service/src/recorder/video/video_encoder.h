#pragma once

#include <filesystem>
#include <memory>
#include "recorder/image/image.h"

namespace service::recorder::video {

class VideoEncoder {
public:
    VideoEncoder(const std::filesystem::path& path):
        _path(path)
    {}
    virtual ~VideoEncoder() {}

    const std::filesystem::path& path() const { return _path; }
    virtual void initialize(size_t width, size_t height) = 0;
    virtual void addVideoFrame(long long frameMs, const service::recorder::image::Image& frame) = 0;

private:
    std::filesystem::path _path;

};
using VideoEncoderPtr = std::unique_ptr<VideoEncoder>;

}