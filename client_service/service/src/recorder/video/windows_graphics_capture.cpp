#ifdef _WIN32

#include "recorder/video/windows_graphics_capture.h"
#include "shared/log/log.h"
#include "shared/system/win32/hwnd_utils.h"
#include "system/settings.h"
#include <VersionHelpers.h>

namespace service::recorder::video {
namespace {

typedef service::recorder::video::VideoRecorder* (__cdecl *WGCPROC)(const service::recorder::video::VideoWindowInfo&, HWND, service::renderer::D3d11SharedContext*, bool);

WGCPROC getLoaderFunc() {
    static HINSTANCE lib = LoadLibrary(TEXT("libwgc.dll"));
    if (!lib) {
        return nullptr;
    }
    static WGCPROC proc = (WGCPROC)GetProcAddress(lib, "createWindowsGraphicsCaptureInterface");
    if (!proc) {
        return nullptr;
    }
    return proc;
}

}

bool tryInitializeWindowsGraphicsCapture(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid, service::renderer::D3d11SharedContext* shared) {
    if (!IsWindows10OrGreater()) {
        LOG_INFO("Rejecting Windows Graphics Capture: Not Windows 10" << std::endl);
        return false;
    }

    HWND wnd = shared::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(0));
    if (!wnd) {
        LOG_INFO("Rejecting Windows Graphics Capture due to inability to find window." << std::endl);
        return false;
    }

    WGCPROC proc = getLoaderFunc();
    if (!proc) {
        LOG_INFO("Rejecting WGC: Can't find creation function." << std::endl);
        return false;
    }

    service::system::getCurrentSettings()->reloadSettingsFromFile();
    const bool useHwFrame = service::system::getCurrentSettings()->recording().useVideoHw;

    auto* ptr = (proc)(info, wnd, shared, useHwFrame);
    if (!ptr) {
        LOG_INFO("Rejecting WGC: Failed to create interface." << std::endl);
        return false;
    }
    output.reset(ptr);
    return true;
}

}
#endif