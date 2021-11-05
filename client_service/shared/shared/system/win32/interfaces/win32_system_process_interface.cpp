#include "shared/system/win32/interfaces/win32_system_process_interface.h"
#include "shared/system/win32/hwnd_utils.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

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

}