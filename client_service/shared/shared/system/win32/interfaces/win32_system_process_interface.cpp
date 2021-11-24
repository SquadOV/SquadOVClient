#include "shared/system/win32/interfaces/win32_system_process_interface.h"
#include "shared/system/win32/hwnd_utils.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/strings/strings.h"
#include "shared/image/ico/ico.h"
#include "shared/base64/encode.h"

#include <Windows.h>
#include <psapi.h>
#include <VersionHelpers.h>

namespace shared::system::win32::interfaces {

std::vector<OSPID> Win32SystemProcessInterface::enumProcesses() const {
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded) ) {
        LOG_WARNING("Failed to enum processes: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return {};
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    std::vector<OSPID> ret(cProcesses);
    for (auto i = 0; i < cProcesses; ++i) {
        ret[i] = aProcesses[i];
    }
    return ret;
}

OSString Win32SystemProcessInterface::getProcessName(OSPID pid) const {
    // WE MUST USE PROCESS_QUERY_INFORMATION | PROCESS_VM_READ HERE.
    // PROCESS_QUERY_INFORMATION and PROCESS_QUERY_LIMITED_INFORMATION by themselves do not work on Windows 7.
    // Not that it really matters since Windows 7 not supported by Microsoft anymore but we had a couple of 
    // users who use it so...can't be picky about our users.
    const DWORD access = IsWindows8Point1OrGreater() ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION;
    HANDLE hProcess = OpenProcess(access, FALSE, pid);

    if (hProcess == NULL) {
        LOG_DEBUG("Failed to get open process: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return L"";
    }

    WCHAR szProcessName[MAX_PATH] = L"<unknown>";
    DWORD processNameSize = sizeof(szProcessName)/sizeof(WCHAR);

    if (QueryFullProcessImageNameW(hProcess, 0, szProcessName, &processNameSize) == 0) {
        CloseHandle(hProcess);
        LOG_DEBUG("Failed to get module filename: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return L"";
    }
    CloseHandle(hProcess);
    return std::wstring(szProcessName);
}

OSWindow Win32SystemProcessInterface::findWindowForProcessWithMaxDelay(OSPID pid, const std::chrono::milliseconds& maxDelayMs, const std::chrono::milliseconds& step, bool quiet, bool checkWindowSize) const {
    return shared::system::win32::findWindowForProcessWithMaxDelay(pid, maxDelayMs, step, quiet, checkWindowSize);
}

OSString Win32SystemProcessInterface::getProcessFriendlyName(const std::filesystem::path& path) const {
    DWORD unk1 = 0;
    const auto fileInfoSize = GetFileVersionInfoSizeExW(0, path.native().c_str(), &unk1);

    std::vector<char> buffer(fileInfoSize);
    if (!GetFileVersionInfoExW(0, path.native().c_str(), 0, fileInfoSize, (void*)buffer.data())) {
        return L"";
    }

    char* infBuffer = nullptr;
    unsigned int infSize = 0;
    if (!VerQueryValueA((void*)buffer.data(), "\\StringFileInfo\\000004B0\\ProductName", (void**)&infBuffer, &infSize)) {
        return L"";
    }

    return shared::strings::utf8ToWcs(std::string(infBuffer, infSize));
}

std::string Win32SystemProcessInterface::getBase64EncodedIconForExe(const std::filesystem::path& path) const {
    HICON icon = ExtractIconW(GetModuleHandle(nullptr), path.native().c_str(), 0);
    if (!icon) {
        return "";
    }

    shared::image::ico::IcoImage image;
    image.addIcon(icon);

    const auto rawBuffer = image.serialize();
    // Unnecessary copy but a little too lazy to write new code to get the interface to match up to the existing base64 encoding.
    const std::string rawBufferStr(rawBuffer.begin(), rawBuffer.end());
    return shared::base64::encode(rawBufferStr, shared::base64::BASE64_ENCODE_DEFAULT_CHARSET);
}

}