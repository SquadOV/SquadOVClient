#pragma once

#include "recorder/encoder/av_encoder.h"

namespace service::recorder::encoder {

class FfmpegAvEncoderImpl;
class FfmpegAvEncoder : public AvEncoder {
public:
    explicit FfmpegAvEncoder(const std::string& streamUrl);
    ~FfmpegAvEncoder();

    const std::string& streamUrl() const override;
    void initializeVideoStream(const service::renderer::D3d11SharedContextPtr&, const service::system::RecordingSettings& settings, size_t width, size_t height) override;
    VideoStreamContext getVideoStreamContext() const override;
    AVSyncClock::time_point getSyncStartTime() const override;

    void finalizeStreams() override;
    void resizeDvrBuffer(double timeSeconds) override;
    void getVideoDimensions(size_t& width, size_t& height) const override;
#ifdef _WIN32
    void addVideoFrame(ID3D11Texture2D* image, size_t numFrames) override;
#endif

    void initializeAudioStream() override;
    size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings) override;
    void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) override;

    shared::TimePoint open(const std::string& outputUrl, std::optional<shared::TimePoint> dvrFillInStartTime) override;
    void start() override;
    void stop() override;
    shared::squadov::VodMetadata getMetadata() const override;
private:
    std::unique_ptr<FfmpegAvEncoderImpl> _impl;
};

}