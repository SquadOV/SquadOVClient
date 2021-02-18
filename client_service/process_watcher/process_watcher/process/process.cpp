#include "process_watcher/process/process.h"

#include "shared/errors/error.h"

#include <algorithm>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <errhandlingapi.h>
#include <winternl.h>
#include <VersionHelpers.h>
#endif

#include <iostream>

namespace {

#ifdef _WIN32

std::unique_ptr<process_watcher::process::Process> createProcess(DWORD id) {
    // WE MUST USE PROCESS_QUERY_INFORMATION | PROCESS_VM_READ HERE.
    // PROCESS_QUERY_INFORMATION and PROCESS_QUERY_LIMITED_INFORMATION by themselves do not work on Windows 7.
    // Not that it really matters since Windows 7 not supported by Microsoft anymore but we had a couple of 
    // users who use it so...can't be picky about our users.
    const DWORD access = IsWindows10OrGreater() ? PROCESS_QUERY_LIMITED_INFORMATION : (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ);
    HANDLE hProcess = OpenProcess(access, FALSE, id);

    if (hProcess == NULL) {
        LOG_DEBUG("Failed to get open process: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return nullptr;
    }

    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    if (GetModuleFileNameEx(hProcess, NULL, szProcessName, sizeof(szProcessName)/sizeof(TCHAR)) == 0) {
        LOG_DEBUG("Failed to get module filename: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return nullptr;
    }
    CloseHandle(hProcess);

    return std::make_unique<process_watcher::process::Process>(std::string(szProcessName), id);
}

#endif

}

namespace process_watcher::process {

bool listRunningProcesses(std::vector<Process>& out) {
#ifdef _WIN32
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded) ) {
        LOG_WARNING("Failed to enum processes: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return false;
    }

    cProcesses = cbNeeded / sizeof(DWORD);
    LOG_DEBUG("Detected " << cProcesses << " Processes using " << cbNeeded << " bytes out of " << sizeof(aProcesses) << std::endl);

    for (DWORD i = 0; i < cProcesses; ++i) {
        if (aProcesses[i] == 0) {
            continue;
        }

        auto ptr = createProcess(aProcesses[i]);
        if (!ptr) {
            continue;
        }
        out.emplace_back(*ptr);
    }
#else
    THROW_ERROR("Unsupported OS for listing processes.");
#endif

    std::sort(out.begin(), out.end(), [](const Process& a, const Process& b) {
        return a.name() < b.name();
    });

    LOG_DEBUG("Final number of processes detected: " << out.size() << std::endl);
    for (const auto& p: out) {
        LOG_DEBUG("\tProcess: " << p.name() << std::endl);
    }
    return true;
}

}