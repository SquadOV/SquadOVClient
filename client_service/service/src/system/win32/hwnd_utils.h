#pragma once

#ifdef _WIN32

#include <chrono>
#include <Windows.h>
#include <ostream>

namespace service::system::win32 {

HWND findWindowForProcessWithMaxDelay(DWORD pid, const std::chrono::milliseconds& maxDelayMs = std::chrono::milliseconds(0));
bool isFullscreen(HWND wnd, HMONITOR monitor, int margin);

}

bool operator==(const RECT& a, const RECT& b);
std::ostream& operator<<(std::ostream& os, const RECT& a);

#endif