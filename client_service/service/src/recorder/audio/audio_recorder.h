#pragma once

#include "recorder/audio/audio_packet_properties.h"
#include "system/settings.h"

#include <filesystem>
#include <memory>
#include <unordered_set>

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

using AudioDeviceSet = std::unordered_set<int>;

class AudioRecorder {
public:
    virtual ~AudioRecorder() {}
    virtual void startRecording() = 0;
    virtual void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) = 0;
    virtual void stop() = 0;
    virtual void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) = 0;
    virtual double initialVolume() const = 0;
    virtual void setVolume(double volume) = 0;

    bool isVoice() const { return _isVoice; }
    void markVoice() { _isVoice = true; }

    virtual bool exists() const = 0;
    virtual const AudioPacketProperties& props() const = 0;

private:
    bool _isVoice = false;
};

using AudioRecorderPtr = std::unique_ptr<AudioRecorder>;

}