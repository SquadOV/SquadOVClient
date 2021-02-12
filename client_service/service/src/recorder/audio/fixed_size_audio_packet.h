#pragma once

#include "recorder/encoder/av_encoder.h"
#include "recorder/audio/audio_packet_properties.h"
#include "recorder/audio/audio_packet_view.h"

#include <array>
#include <cassert>

namespace service::recorder::audio {

// N is the total capacity of this packet.
// The actual "size" of this packet in terms of number of samples can be found by
// accessing the props property.
template<typename T, size_t N>
class FixedSizeAudioPacket {
public:
    static const size_t N = N;
    using SyncTime = service::recorder::encoder::AVSyncClock::time_point;

    FixedSizeAudioPacket() = default;

    FixedSizeAudioPacket(const AudioPacketProperties& props, const SyncTime& tm):
        _tm(tm),
        _props(props) {
    }

    size_t capacity() const { return N; }
    T* buffer() { return _buffer.data(); }
    const T* buffer() const { return _buffer.data(); }
    const AudioPacketProperties& props() const { return _props; }
    const SyncTime& syncTime() const { return _tm; }
    
    void copyFrom(const AudioPacketView<T>& view, size_t start = 0, size_t end = N) {
        assert((end - start) == (props().numSamples * props().numChannels));
        size_t bi = 0;
        for (size_t i = start; i < end; ++i) {
            _buffer[bi++] = view.buffer()[i];
        }
    }

private:
    std::array<T, N> _buffer;
    SyncTime _tm;
    AudioPacketProperties _props;
};

template<size_t N>
using FFixedSizeAudioPacket = FixedSizeAudioPacket<float, N>;

}