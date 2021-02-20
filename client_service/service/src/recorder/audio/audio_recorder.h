#pragma once

#include "recorder/audio/audio_packet_properties.h"

#include <filesystem>
#include <memory>

namespace service::recorder::encoder {
class AvEncoder;
}

namespace service::recorder::audio {

enum class EAudioDeviceDirection {
    Input,
    Output
};

inline std::string audioDeviceDirectionToStr(EAudioDeviceDirection dir) {
    switch (dir) {
    case EAudioDeviceDirection::Input:
        return "INPUT";
    case EAudioDeviceDirection::Output:
        return "OUTPUT";
    }
    return "";
}

class AudioRecorder {
public:
    virtual ~AudioRecorder() {}
    virtual void startRecording() = 0;
    virtual void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) = 0;
    virtual void stop() = 0;
    virtual void loadDevice(EAudioDeviceDirection dir, const std::string& selected, double volume) = 0;

    virtual bool exists() const = 0;
    virtual const AudioPacketProperties& props() const = 0;
};

using AudioRecorderPtr = std::unique_ptr<AudioRecorder>;

}