#pragma once

#include "recorder/encoder/av_encoder.h"

namespace service::recorder::encoder {

class FfmpegAvEncoderImpl;
class FfmpegAvEncoder : public AvEncoder {
public:
    explicit FfmpegAvEncoder(const std::filesystem::path& path);
    ~FfmpegAvEncoder();

    const std::filesystem::path& path() const override;
    void initializeVideoStream(size_t fps, size_t width, size_t height) override;
    void getVideoDimensions(size_t& width, size_t& height) const override;
    void addVideoFrame(const service::recorder::image::Image& frame) override;

    void initializeAudioStream() override;
    size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps) override;
    void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) override;

    void start() override;
    void stop() override;
private:
    std::unique_ptr<FfmpegAvEncoderImpl> _impl;
};

}