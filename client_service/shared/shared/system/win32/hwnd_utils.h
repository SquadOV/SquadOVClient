#pragma once

#ifdef _WIN32

#include <chrono>
#include <Windows.h>
#include <ostream>

namespace shared::system::win32 {

HWND findWindowForProcessWithMaxDelay(DWORD pid, const std::chrono::milliseconds& maxDelayMs = std::chrono::milliseconds(0), const std::chrono::milliseconds& step = std::chrono::milliseconds(1000), bool quiet = false);
bool isWindowTopmost(HWND wnd);
bool isFullscreen(HWND wnd, HMONITOR monitor, int margin);
bool isProcessForeground(DWORD pid);

}

bool operator==(const RECT& a, const RECT& b);
std::ostream& operator<<(std::ostream& os, const RECT& a);

#endif