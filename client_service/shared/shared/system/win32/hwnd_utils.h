#pragma once

#ifdef _WIN32

#include <chrono>
#include <Windows.h>
#include <ostream>
#include <optional>

namespace shared::system::win32 {

HWND findWindowForProcessWithMaxDelay(DWORD pid, const std::chrono::milliseconds& maxDelayMs = std::chrono::milliseconds(0), const std::chrono::milliseconds& step = std::chrono::milliseconds(1000), bool quiet = false, bool checkWindowSize = true);
bool isWindowTopmost(HWND wnd);
bool isFullscreen(HWND wnd, HMONITOR monitor, int margin);
bool isProcessForeground(DWORD pid);
bool isHDREnabledForMonitor(HMONITOR monitor);
std::optional<double> getNativeMonitorAspectRatio(HMONITOR monitor);
std::optional<double> getNativeMonitorAspectRatioFromDeviceIdAndClassGuid(const std::string& deviceId, GUID guid);
std::optional<double> getNativeMonitorAspectRatioFromEDID(HKEY regKey);

}

std::ostream& operator<<(std::ostream& os, const RECT& a);

#endif