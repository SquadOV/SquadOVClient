#ifdef _WIN32

#include "recorder/video/windows_graphics_capture.h"
#include "shared/log/log.h"
#include "system/win32/hwnd_utils.h"
#include <VersionHelpers.h>

namespace service::recorder::video {
namespace {

typedef service::recorder::video::VideoRecorder* (__cdecl *WGCPROC)(const service::recorder::video::VideoWindowInfo&, HWND, service::renderer::D3d11SharedContext*);

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

    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(120000));
    if (!wnd) {
        LOG_INFO("Rejecting Windows Graphics Capture due to inability to find window." << std::endl);
        return false;
    }

    WGCPROC proc = getLoaderFunc();
    if (!proc) {
        LOG_INFO("Rejecting WGC: Can't find creation function." << std::endl);
        return false;
    }

    auto* ptr = (proc)(info, wnd, shared);
    if (!ptr) {
        LOG_INFO("Rejecting WGC: Failed to create interface." << std::endl);
        return false;
    }
    output.reset(ptr);
    return true;
}

}
#endif