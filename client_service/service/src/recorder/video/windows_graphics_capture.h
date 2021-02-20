#pragma once

#include "recorder/video/video_recorder.h"

#ifdef _WIN32
#include <Windows.h>

namespace service::recorder::video {

bool tryInitializeWindowsGraphicsCapture(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid);

}
#endif