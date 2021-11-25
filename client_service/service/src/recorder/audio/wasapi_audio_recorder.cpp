#include "recorder/audio/wasapi_audio_recorder.h"
#include "recorder/audio/win32/wasapi_interface.h"
#include "shared/audio/timing.h"
#include "shared/strings/strings.h"
#include "shared/log/log.h"
#include "recorder/audio/wasapi_audio_client_recorder.h"

#include <atlbase.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

namespace service::recorder::audio {
class WasapiAudioRecorderImpl {
public:
    ~WasapiAudioRecorderImpl();

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
    void stop();
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet);
    double initialVolume() const { return _initialVolume; }
    void setVolume(double volume) { _internal->setVolume(volume); }

    bool exists() const { return _internal && _internal->exists(); }
    const AudioPacketProperties& props() const { return _internal->props(); }

private:
    void printWarning(const std::string& msg, HRESULT hr) const;

    double _initialVolume = 1.0;
    service::system::AudioDeviceSettings _inDevice;

    // WASAPI recording
    CComPtr<IMMDevice> _device;
    WasapiAudioClientRecorderPtr _internal;
};

WasapiAudioRecorderImpl::~WasapiAudioRecorderImpl() {
}

void WasapiAudioRecorderImpl::printWarning(const std::string& msg, HRESULT hr) const {
    LOG_WARNING(msg << ": " << hr << " for [" << _inDevice.device << " (" << _inDevice.id << ")]" << std::endl);
}

void WasapiAudioRecorderImpl::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    LOG_INFO("Load Selected Audio Device: '" << device.device << "(" << device.id << ")" << "' @ " << device.volume << " [Mono: " << device.mono << "]" << std::endl);
    _inDevice = device;

    bool isDefault = false;
    // The input device contains the ID - we only use the input dir if the passed in device is the default device.
    if (device.id.empty()) {
        _device = service::recorder::audio::win32::WASAPIInterface::getDefaultDevice(dir);
        isDefault = true;
    } else {
        _device = service::recorder::audio::win32::WASAPIInterface::getDeviceFromId(device.id);
    }

    if (!_device) {
        printWarning("...Failed to find audio device.", 0);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    std::string selectedDeviceId;
    HRESULT hr = S_OK;
    {
        LPWSTR deviceId = nullptr;
        HRESULT hr = _device->GetId(&deviceId);
        if (hr != S_OK) {
            printWarning("...Failed to get ID", hr);
            if (!isDefault) {
                loadDevice(dir, device.createDefault(), deviceSet);
            }
            return;
        }
        selectedDeviceId = shared::strings::wcsToUtf8(deviceId);
        CoTaskMemFree(deviceId);
    }

    if (deviceSet.find(selectedDeviceId) != deviceSet.end()) {
        printWarning("...Duplicate device detected. Ignoring", 0);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    CComPtr<IAudioClient> audioClient;
    hr = _device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&audioClient
    );

    if (hr != S_OK) {
        printWarning("...Failed to get audio client", hr);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    _internal = std::make_unique<WasapiAudioClientRecorder>(
        audioClient,
        service::recorder::audio::win32::WASAPIInterface::getDeviceName(_device),
        device.mono,
        dir == EAudioDeviceDirection::Output
    );

    if (_internal->exists()) {
        deviceSet.insert(selectedDeviceId);
    } else if (!isDefault) {
        loadDevice(dir, device.createDefault(), deviceSet);
    }
}

void WasapiAudioRecorderImpl::startRecording() {
    _internal->startRecording();
}

void WasapiAudioRecorderImpl::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _internal->setActiveEncoder(encoder, encoderIndex);
}

void WasapiAudioRecorderImpl::stop() {
    _internal->stop();
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