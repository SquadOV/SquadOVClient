#include "system/win32/hwnd_utils.h"

#ifdef _WIN32

#include <thread>
#include <iostream>

namespace service::system::win32 {
namespace {

struct EnumData {
    DWORD pid;
    HWND out;
    bool found = false;
};

BOOL enumWindowCallback(HWND hwnd, LPARAM param) {
    DWORD refPid;
    EnumData* data = (EnumData*)param;
    GetWindowThreadProcessId(hwnd, &refPid);

    if (refPid == data->pid) {
        data->out = hwnd;
        data->found = true;
        return FALSE;
    } else {
        return TRUE;
    }
}

}

HWND findWindowForProcessWithMaxDelay(DWORD pid, const std::chrono::milliseconds& maxDelayMs) {
    // Need to first find the window associated with the process.
    EnumData window;
    window.pid = pid;

    // If we don't find the window, we can afford to wait a little bit before
    // determining the window can't be found as the user might have just
    // started the game so it's still in the process of creating the window.
    auto delay = std::chrono::milliseconds(0);
    const auto step = std::chrono::milliseconds(1000);

    while (delay < maxDelayMs) {
        EnumWindows(enumWindowCallback, (LPARAM)&window);
        if (window.found) {
            return window.out;
        }

        std::this_thread::sleep_for(step);
        delay += step;
    }
    
    return NULL;
}
bool isFullscreen(HWND wnd, HMONITOR monitor) {
    RECT hwndRect;
    if(!GetWindowRect(wnd, &hwndRect)) {
        return false;
    }
    
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoA(monitor, &info)) {
        return false;
    }
    return hwndRect == info.rcMonitor;
}

}

bool operator==(const RECT& a, const RECT& b) {
    return a.left == b.left && a.right == b.right && a.top == b.top && a.bottom == b.bottom;   
}

std::ostream& operator<<(std::ostream& os, const RECT& a) {
    os << "RECT: " << a.bottom << " " << a.left << " " << a.top << " " << a.right;
    return os;
}

#endif