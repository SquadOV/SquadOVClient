#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/device.h"

#ifdef _WIN32

#include <wil/com.h>
#include <mmdeviceapi.h>

namespace service::recorder::audio::win32 {

class WASAPIInterface {
public:
    static service::recorder::audio::AudioDeviceResponse getDeviceListing(service::recorder::audio::EAudioDeviceDirection dir);
    static wil::com_ptr<IMMDevice> getDeviceFromName(service::recorder::audio::EAudioDeviceDirection dir, const std::string& name);
    static wil::com_ptr<IMMDevice> getDeviceFromId(const std::string& id);
    static wil::com_ptr<IMMDevice> getDefaultDevice(service::recorder::audio::EAudioDeviceDirection dir);
    static std::string getDeviceName(IMMDevice* device);
private:
};

}

#endif