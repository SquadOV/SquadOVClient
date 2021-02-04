#pragma once

#include "recorder/audio/audio_packet_properties.h"

namespace service::recorder::audio {

// A generalized representation of an "audio packet" which is an amalgamation
// of terms from both Portaudio and Ffmpeg. Generally how audio works is that you sample
// the audio signal at a certain rate (sampling rate) each of these values can be represented
// using some number (signed integer 32 bits, float, etc.). We'll refer to each of these values
// as a "sample" which is the term FFmpeg uses; Portaudio refers to them as a frame. Generally
// we'll get a group of these samples all at once. Portaudio will give them to us in a "frame buffer"
// while FFmpeg refers to them as a "frame." AudioPacketView is a generalized representation of
// a number of samples. Thus to compute the number of frames in this AudioPacketView you need to
// multiply the number of samples per channel (numSamples) by the number of channels (numChannels).
template<typename T>
class AudioPacketView {
public:
    AudioPacketView(const T* buffer, const AudioPacketProperties& props):
        _buffer(buffer),
        _props(props) {
    }

    const T* buffer() const { return _buffer; }
    const AudioPacketProperties& props() const { return _props; }

private:
    const T* _buffer = nullptr;
    const AudioPacketProperties _props;
};

using FAudioPacketView = AudioPacketView<float>;
}