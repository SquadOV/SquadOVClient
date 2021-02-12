#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include "recorder/image/image.h"
#include "recorder/audio/audio_packet_view.h"
#include "shared/squadov/vod.h"

namespace service::recorder::encoder {

using AVSyncClock = std::chrono::high_resolution_clock;

struct AudioStreamProperties {
    size_t encoderIndex = 0;
    service::recorder::audio::AudioPacketProperties props;
};

class AvEncoder {
public:
    virtual ~AvEncoder() {}

    virtual const std::string& streamUrl() const = 0;
    virtual void initializeVideoStream(size_t fps, size_t width, size_t height) = 0;
    virtual void addVideoFrame(const service::recorder::image::Image& frame) = 0;
    virtual void getVideoDimensions(size_t& width, size_t& height) const = 0;
    virtual service::recorder::image::Image getFrontBuffer() const = 0;

    virtual void initializeAudioStream() = 0;
    virtual size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps) = 0;
    virtual void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) = 0;

    virtual void open() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual shared::squadov::VodMetadata getMetadata() const = 0;
};
using AvEncoderPtr = std::unique_ptr<AvEncoder>;

}