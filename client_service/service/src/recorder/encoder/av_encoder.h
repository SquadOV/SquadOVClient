#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include "recorder/image/image.h"
#include "recorder/audio/audio_packet_view.h"
#include "renderer/d3d11_context.h"
#include "shared/squadov/vod.h"
#include "shared/time.h"
#include "recorder/encoder/av_sync.h"
#include "system/settings.h"

#ifdef _WIN32
#include <d3d11.h>
#endif

namespace service::recorder::encoder {

struct AudioStreamProperties {
    size_t encoderIndex = 0;
    service::recorder::audio::AudioPacketProperties props;
};

struct AudioInputSettings {
    bool useSilenceCompensation = false;
    bool useNoiseThreshold = false;
    int32_t noiseThresholDb = -60;
    bool useSpeechNoiseReduction = false;
};

enum class VideoStreamContext {
    CPU,
    GPU
};

class AvEncoder {
public:
    virtual ~AvEncoder() {}

    virtual const std::string& streamUrl() const = 0;
    virtual void initializeVideoStream(const service::renderer::D3d11SharedContextPtr& d3d, const service::system::RecordingSettings& settings, size_t width, size_t height) = 0;
    virtual VideoStreamContext getVideoStreamContext() const = 0;
    virtual AVSyncClock::time_point getSyncStartTime() const = 0;
    
    virtual void finalizeStreams() = 0;
    virtual void resizeDvrBuffer(double timeSeconds) = 0;
#ifdef _WIN32
    virtual void addVideoFrame(ID3D11Texture2D* image, size_t numFrames) = 0;
#endif

    virtual void getVideoDimensions(size_t& width, size_t& height) const = 0;

    virtual void initializeAudioCodec() = 0;
    virtual void initializeAudioStreams(bool separateTracks) = 0;
    virtual size_t addAudioInput(const std::string& name, const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings) = 0;
    virtual void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) = 0;

    virtual shared::TimePoint open(const std::string& outputUrl, std::optional<AVSyncClock::time_point> dvrFillInStartTime) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual shared::squadov::VodMetadata getMetadata() const = 0;
};
using AvEncoderPtr = std::unique_ptr<AvEncoder>;

}