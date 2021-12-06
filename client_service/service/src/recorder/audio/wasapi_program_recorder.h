#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/device.h"

namespace service::recorder::audio {

class WasapiProgramRecorderImpl;

// This class is responsible for recording audio from PROGRAMS.
// The interface for recording loopback from devices is separate.
class WasapiProgramRecorder : public AudioRecorder {
public:
    WasapiProgramRecorder(OSPID pid, double volume);
    ~WasapiProgramRecorder();

    void startRecording() override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) override;
    void stop() override;
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) override;
    double initialVolume() const override;
    void setVolume(double volume) override;

    bool exists() const override;
    const AudioPacketProperties& props() const override;
private:
    std::unique_ptr<WasapiProgramRecorderImpl> _impl;
};

}