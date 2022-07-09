#pragma once

#include <boost/circular_buffer.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <optional>
#include <deque>
#include <vector>
#include <unordered_map>
#include "recorder/encoder/av_sync.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
}

namespace service::recorder::audio {

class FfmpegAudioBuffer {
public:
    FfmpegAudioBuffer(AVSampleFormat format, int channels, int sampleRate);

    void startBuffer();
    void addFrame(AVFrame* frame, size_t inputIndex, const service::recorder::encoder::AVSyncClock::time_point& tm);
    bool hasFrameOlderThanMs(size_t samples, const std::chrono::milliseconds& ms);
    void popFrameInto(AVFrame* output);

private:
    std::mutex _mutex;
    AVSampleFormat _format;
    int _channels = 0;
    bool _isPlanar = false;
    int _sampleRate = 0;

    struct Buffer {
        boost::circular_buffer<uint8_t> data;
        std::optional<service::recorder::encoder::AVSyncClock::time_point> startTm;
        std::unordered_map<size_t, size_t> desiredNextIndex;
    };

    std::vector<Buffer> _buffer;
    int _bytesPerSample = 0;
};

using FfmpegAudioBufferPtr = std::shared_ptr<FfmpegAudioBuffer>;

}