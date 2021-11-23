#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/device.h"

#ifdef _WIN32

#include <atlbase.h>
#include <mmdeviceapi.h>

namespace service::recorder::audio::win32 {

class WASAPIInterface {
public:
    static service::recorder::audio::AudioDeviceResponse getDeviceListing(service::recorder::audio::EAudioDeviceDirection dir);
    static CComPtr<IMMDevice> getDeviceFromName(service::recorder::audio::EAudioDeviceDirection dir, const std::string& name);
    static CComPtr<IMMDevice> getDeviceFromId(const std::string& id);
    static CComPtr<IMMDevice> getDefaultDevice(service::recorder::audio::EAudioDeviceDirection dir);
    static std::string getDeviceName(IMMDevice* device);
private:
};

}

#endif