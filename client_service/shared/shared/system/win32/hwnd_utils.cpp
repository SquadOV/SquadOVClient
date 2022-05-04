#include "shared/system/win32/hwnd_utils.h"

#ifdef _WIN32

#include <atlbase.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <thread>
#include <iostream>
#include <VersionHelpers.h>
#include <wil/com.h>
#include <SetupAPI.h>

#include "shared/log/log.h"
#include "shared/errors/error.h"

#pragma comment(lib, "dxgi")
#pragma comment(lib, "setupapi.lib")

namespace shared::system::win32 {
namespace {

constexpr int MINIMUM_WINDOW_RESOLUTION_XY = 100;

const GUID GUID_CLASS_MONITOR = {
    0x4d36e96e,
    0xe325,
    0x11ce,
    0xbf,
    0xc1,
    0x08,
    0x00,
    0x2b,
    0xe1,
    0x03,
    0x18
};

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

    LOG_INFO("Check Full screen: " << std::endl
        << "\tMonitor: " << monitorWidth << "x" << monitorHeight << std::endl
        << "\tWindow: " << wndWidth << "x" << wndHeight << std::endl
    );
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

std::optional<double> getNativeMonitorAspectRatio(HMONITOR monitor) {
    // First get the monitor device name.
    MONITORINFOEX info = { 0 };
    info.cbSize = sizeof(MONITORINFOEX);

    if (!GetMonitorInfo(monitor, &info)) {
        return std::nullopt;
    }

    const std::string refDeviceName(info.szDevice);

    // Then iterate through display devices and find the display device with the equivalent device name.
    DISPLAY_DEVICE dd = { 0 };
    dd.cb = sizeof(DISPLAY_DEVICE);
    if (!EnumDisplayDevices(refDeviceName.c_str(), 0, &dd, 0)) {
        return std::nullopt;
    }

    // The DeviceID field of DISPLAY_DEVICE will be something like MONITOR\\{{DEVICE ID}}\\{CLASS GUID}\\{{INSTANCE ID}}.
    // We want to extract BLAH which will be the get the actual DeviceID that we can use to search the registry
    // for the information we want.
    const std::string fullDeviceId(dd.DeviceID);
    const auto idSep = fullDeviceId.find("\\");
    const auto idSepNext = fullDeviceId.find("\\", idSep + 1);
    if (idSep == std::string::npos || idSepNext == std::string::npos) {
        return std::nullopt;
    }

    const auto guidSep = fullDeviceId.find("{");
    const auto guidSepNext = fullDeviceId.find("}", guidSep + 1);
    if (guidSep == std::string::npos || guidSepNext == std::string::npos) {
        return std::nullopt;
    }

    const std::string guidStr = fullDeviceId.substr(guidSep, guidSepNext - guidSep + 1);
    GUID guid;
    if (CLSIDFromString(CComBSTR(guidStr.c_str()), (LPCLSID)&guid) != S_OK) {
        return std::nullopt;
    }
    const std::string deviceId = fullDeviceId.substr(idSep + 1, idSepNext - idSep - 1);
    return getNativeMonitorAspectRatioFromDeviceIdAndClassGuid(deviceId, guid);
}

std::optional<double> getNativeMonitorAspectRatioFromDeviceIdAndClassGuid(const std::string& deviceId, GUID guid) {
    HDEVINFO devInfo = SetupDiGetClassDevsEx(
        &GUID_CLASS_MONITOR,
        NULL,
        NULL,
        DIGCF_ALLCLASSES | DIGCF_PRESENT,
        NULL,
        NULL,
        NULL
    );

    if (!devInfo) {
        return std::nullopt;
    }

    // Iterate through the device information set and open up each setup api registry key
    // to scrape the resolution from it.
    DWORD index = 0;
    SP_DEVINFO_DATA data = { 0 };
    data.cbSize = sizeof(SP_DEVINFO_DATA);

    std::optional<double> aspectRatio;
    while (!aspectRatio && SetupDiEnumDeviceInfo(devInfo, index++, &data)) {
        // If the class GUID isn't equal then we can ignore.
        if (!IsEqualGUID(guid, data.ClassGuid)) {
            continue;
        }

        // Get the device instance ID.
        char testDeviceId[2048];
        if (!SetupDiGetDeviceInstanceId(devInfo, &data, testDeviceId, sizeof(testDeviceId), nullptr)) {
            continue;
        }

        // The device instance ID is slightly different from the device instance ID from DISPLAY_DEVICe structure.
        // But there's a common 7 letter identifier for the monitor which is what we pass in via deviceId.
        if (std::string(testDeviceId).find(deviceId) == std::string::npos) {
            continue;
        }

        // Open the registry key for the device.
        HKEY key = SetupDiOpenDevRegKey(devInfo, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if (!key || key == INVALID_HANDLE_VALUE) {
            continue;
        }

        aspectRatio = getNativeMonitorAspectRatioFromEDID(key);
        RegCloseKey(key);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
    return aspectRatio;
}

std::optional<double> getNativeMonitorAspectRatioFromEDID(HKEY regKey) {
    // Iterate through the all the registry key and values to find the one that's the EDID.
    // Parse the EDID to get the width and height of the monitor and use that to compute the aspect ratio.
    DWORD index = 0;

    char keyName[16384];
    DWORD keyLength = 16383;

    BYTE edid[1024];
    DWORD edidLength = 1024;
    while (RegEnumValue(regKey, index++, keyName, &keyLength, NULL, NULL, edid, &edidLength) == ERROR_SUCCESS) {
        const std::string keyNameStr(keyName);
        if (keyNameStr != "EDID") {
            continue;
        }

        // 66: horizontal image size (mm, 8 least significant bits)
        // 67: vertical image size (mm, 8 least signiicant bits)
        // 68:
        //     - bits 7-4: horizontal image size (mm, 4 most significant bits)
        //     - bits 3-0: vertical image size (mm, 4 most significant bits)
        const auto width = ((edid[68] & 0xF0) << 4) + edid[66];
        const auto height = ((edid[68] & 0x0F) << 8) + edid[67];
        return static_cast<double>(width) / height;
    }
    return std::nullopt;
}

}

std::ostream& operator<<(std::ostream& os, const RECT& a) {
    os << "RECT: " << a.bottom << " " << a.left << " " << a.top << " " << a.right;
    return os;
}

#endif