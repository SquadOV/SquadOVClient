#pragma once

#include "recorder/audio/audio_recorder.h"

namespace service::recorder::audio {

class PortaudioAudioRecorderImpl;
class PortaudioAudioRecorder : public AudioRecorder {
public:
    PortaudioAudioRecorder(EAudioDeviceDirection dir);
    ~PortaudioAudioRecorder();

    void startRecording(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) override;
    void stop() override;

    bool exists() const override;
    const AudioPacketProperties& props() const override;
private:
    std::unique_ptr<PortaudioAudioRecorderImpl> _impl;
};

}