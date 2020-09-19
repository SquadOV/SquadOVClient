#pragma once

#include "recorder/video/video_encoder.h"

namespace service::recorder::video {

class FfmpegVideoEncoderImpl;
class FfmpegVideoEncoder : public VideoEncoder {
public:
    FfmpegVideoEncoder(const std::filesystem::path& path);
    ~FfmpegVideoEncoder();

    void initialize(size_t width, size_t height) override;
    void addVideoFrame(long long frameMs, const service::recorder::image::Image& frame) override;

private:
    std::unique_ptr<FfmpegVideoEncoderImpl> _impl;
};

}