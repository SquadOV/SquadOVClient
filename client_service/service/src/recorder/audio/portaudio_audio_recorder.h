#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/device.h"

namespace service::recorder::audio {

class PortaudioAudioRecorderImpl;
class PortaudioAudioRecorder : public AudioRecorder {
public:
    PortaudioAudioRecorder();
    ~PortaudioAudioRecorder();

    static AudioDeviceResponse getDeviceListing(EAudioDeviceDirection dir);

    void startRecording() override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) override;
    void stop() override;
    void loadDevice(EAudioDeviceDirection dir, const std::string& selected, double volume) override;
    void setVolume(double volume) override;

    bool exists() const override;
    const AudioPacketProperties& props() const override;
private:
    std::unique_ptr<PortaudioAudioRecorderImpl> _impl;
};

}