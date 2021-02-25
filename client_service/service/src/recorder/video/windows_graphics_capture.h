#pragma once

#include "recorder/video/video_recorder.h"
#include "renderer/d3d11_context.h"

#ifdef _WIN32
#include <Windows.h>

namespace service::recorder::video {

bool tryInitializeWindowsGraphicsCapture(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid, service::renderer::D3d11SharedContext* shared);

}
#endif