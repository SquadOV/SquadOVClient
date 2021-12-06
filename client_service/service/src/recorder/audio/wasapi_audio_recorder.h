#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/device.h"

namespace service::recorder::audio {

class WasapiAudioRecorderImpl;

// This class is responsible for recording audio from DEVICES.
// The interface for recording loopback from processes is separate.
class WasapiAudioRecorder : public AudioRecorder {
public:
    WasapiAudioRecorder();
    ~WasapiAudioRecorder();

    void startRecording() override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) override;
    void stop() override;
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) override;
    double initialVolume() const override;
    void setVolume(double volume) override;

    bool exists() const override;
    const AudioPacketProperties& props() const override;
private:
    std::unique_ptr<WasapiAudioRecorderImpl> _impl;
};

}