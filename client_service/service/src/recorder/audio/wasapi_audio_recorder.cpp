#include "recorder/audio/wasapi_audio_recorder.h"

#include <atomic>
#include <atlbase.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

namespace service::recorder::audio {

class WasapiAudioRecorderImpl {
public:
    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
    void stop();
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet);
    double initialVolume() const { return _initialVolume; }
    void setVolume(double volume) { _volume = volume; }

    bool exists() const { return _exists; }
    const AudioPacketProperties& props() const { return _props; }

private:
    EAudioDeviceDirection _dir;
    bool _exists = false;
    std::atomic<double> _volume = 1.0;
    double _initialVolume = 1.0;
    bool _mono = false;
    AudioPacketProperties _props;

    // WASAPI recording
    CComPtr<IMMDevice> _device;
    CComPtr<IAudioClient> _audioClient;
    CComPtr<IAudioCaptureClient> _captureclient;
};

void WasapiAudioRecorderImpl::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
}

void WasapiAudioRecorderImpl::startRecording() {

}

void WasapiAudioRecorderImpl::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {

}

void WasapiAudioRecorderImpl::stop() {

}

WasapiAudioRecorder::WasapiAudioRecorder():
    _impl(new WasapiAudioRecorderImpl)
{
}

WasapiAudioRecorder::~WasapiAudioRecorder() {

}

void WasapiAudioRecorder::startRecording() {
    _impl->startRecording();
}

void WasapiAudioRecorder::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _impl->setActiveEncoder(encoder, encoderIndex);
}

void WasapiAudioRecorder::stop() {
    _impl->stop();
}

void WasapiAudioRecorder::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    _impl->loadDevice(dir, device, deviceSet);
}

double WasapiAudioRecorder::initialVolume() const {
    return _impl->initialVolume();
}

void WasapiAudioRecorder::setVolume(double volume) {
    _impl->setVolume(volume);
}

bool WasapiAudioRecorder::exists() const {
    return _impl->exists();
}

const AudioPacketProperties& WasapiAudioRecorder::props() const {
    return _impl->props();
}

}