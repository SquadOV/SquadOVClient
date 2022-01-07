#include "shared/system/win32/hwnd_utils.h"

#ifdef _WIN32

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <thread>
#include <iostream>
#include <VersionHelpers.h>
#include <wil/com.h>

#include "shared/log/log.h"
#include "shared/errors/error.h"

#pragma comment(lib, "dxgi")

namespace shared::system::win32 {
namespace {

constexpr int MINIMUM_WINDOW_RESOLUTION_XY = 100;

struct EnumData {
    DWORD pid;
    HWND out;
    bool found = false;
    bool checkWindowSize = false;
};

BOOL enumWindowCallback(HWND hwnd, LPARAM param) {
    DWORD refPid;
    EnumData* data = (EnumData*)param;
    GetWindowThreadProcessId(hwnd, &refPid);

    if (refPid == data->pid && IsWindowEnabled(hwnd) && IsWindowVisible(hwnd)) {
        RECT windowRes;
        if (!GetClientRect(hwnd, &windowRes)) {
            return TRUE;
        }

        const auto width = windowRes.right - windowRes.left;
        const auto height = windowRes.bottom - windowRes.top;

        if (!data->checkWindowSize || (width >= MINIMUM_WINDOW_RESOLUTION_XY && height >= MINIMUM_WINDOW_RESOLUTION_XY)) {
            data->out = hwnd;
            data->found = true;
            return FALSE;
        } else {
            return TRUE;
        }
    } else {
        return TRUE;
    }
}

}

HWND findWindowForProcessWithMaxDelay(DWORD pid, const std::chrono::milliseconds& maxDelayMs, const std::chrono::milliseconds& step, bool quiet, bool checkWindowSize) {
    // Need to first find the window associated with the process.
    EnumData window;
    window.pid = pid;
    window.checkWindowSize = checkWindowSize;

    // If we don't find the window, we can afford to wait a little bit before
    // determining the window can't be found as the user might have just
    // started the game so it's still in the process of creating the window.
    auto delay = std::chrono::milliseconds(0);

    const DWORD access = IsWindows8Point1OrGreater() ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION;
    HANDLE pHandle = OpenProcess(access, FALSE, pid);
    if (!pHandle) {
        if (!quiet) {
            THROW_WIN32_ERROR("Failed to open process for window finding.");
        }
        return NULL;
    }

    while (delay < maxDelayMs || maxDelayMs.count() == 0) {
        EnumWindows(enumWindowCallback, (LPARAM)&window);
        if (window.found) {
            char windowTitle[1024];
            GetWindowTextA(window.out, windowTitle, 1024);
            if (!quiet) {
                LOG_INFO("Found Window for Process: " << windowTitle << std::endl);
            }

            CloseHandle(pHandle);
            return window.out;
        }

        if (step.count() == 0) {
            CloseHandle(pHandle);
            return NULL;
        }

        std::this_thread::sleep_for(step);
        delay += step;

        // If the process stopped then we need to stop looking for its window.
        DWORD exitCode = 0;
        auto success = GetExitCodeProcess(pHandle, &exitCode);
        if (!success) {
            LOG_WARNING("Failed to obtain exit code - assuming process stopped." << std::endl);
            break;
        } else if (exitCode != STILL_ACTIVE) {
            LOG_WARNING("Received a non-STILL_ACTIVE exit code - assuming process stopped with exit code :: " << exitCode << std::endl);
            break;
        }
    }
    
    CloseHandle(pHandle);
    return NULL;
}

bool isWindowTopmost(HWND wnd) {
    HWND refWnd = GetForegroundWindow();
    return wnd == refWnd;
}

bool isProcessForeground(DWORD pid) {
    HWND refWnd = GetForegroundWindow();
    if (!refWnd) {
        return false;
    }

    DWORD testPid;
    if (!GetWindowThreadProcessId(refWnd, &testPid)) {
        return false;
    }

    return (testPid == pid);
}

bool isFullscreen(HWND wnd, HMONITOR monitor, int margin) {
    RECT hwndRect;
    if(!GetWindowRect(wnd, &hwndRect)) {
        return false;
    }
    
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoA(monitor, &info)) {
        return false;
    }

    const auto wndWidth = hwndRect.right - hwndRect.left;
    const auto wndHeight = hwndRect.bottom - hwndRect.top;

    const auto monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
    const auto monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

    return (std::abs(wndWidth - monitorWidth) <= margin) &&
        (std::abs(wndHeight - monitorHeight) <= margin);
}

bool isHDREnabledForMonitor(HMONITOR monitor) {
    if (!IsWindows10OrGreater()) {
        return false;
    }

    wil::com_ptr<IDXGIFactory1> factory;
    wil::com_ptr<IDXGIAdapter1> adapter;
    HRESULT hr = S_OK;
    UINT lastAdapterIndex = 0;

    hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));
    if (hr != S_OK) {
        LOG_ERROR("Failed to create DXGI factory [HDR monitor check]: " << hr);
        return false;
    }

    while (factory->EnumAdapters1(lastAdapterIndex++, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        DXGI_OUTPUT_DESC outputDesc;
        UINT outputIndex = 0;
        wil::com_ptr<IDXGIOutput> dxgiOutput;

        while (adapter->EnumOutputs(outputIndex++, &dxgiOutput) != DXGI_ERROR_NOT_FOUND) {
            hr = dxgiOutput->GetDesc(&outputDesc);
            if (hr != S_OK) {
                continue;
            }

            if (outputDesc.Monitor != monitor) {
                continue;
            }

            // At this point we've find the adapter/output combination for the monitor.
            // We want to get the IDXGIOutput6 interface to call GetDesc1 to get the monitor's
            // color space and use that to determine whether or not the monitor is using HDR.
            wil::com_ptr<IDXGIOutput6> dxgiOutput6;
            hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput6), (void**)&dxgiOutput6);
            if (hr != S_OK) {
                LOG_WARNING("Failed to get IDXGIOutput6...assuming no HDR support: " << hr << std::endl);
                return false;
            }

            DXGI_OUTPUT_DESC1 odesc;
            dxgiOutput6->GetDesc1(&odesc);

            if (odesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) {
                return false;
            } else {
                return true;
            }
        }
    }

    return false;
}

}

std::ostream& operator<<(std::ostream& os, const RECT& a) {
    os << "RECT: " << a.bottom << " " << a.left << " " << a.top << " " << a.right;
    return os;
}

#endif