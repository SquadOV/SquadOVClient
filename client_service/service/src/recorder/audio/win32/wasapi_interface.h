#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/device.h"

#ifdef _WIN32

namespace service::recorder::audio::win32 {

class WASAPIInterface {
public:
    static service::recorder::audio::AudioDeviceResponse getDeviceListing(service::recorder::audio::EAudioDeviceDirection dir);

private:
};

}

#endif