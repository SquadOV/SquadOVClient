#include "process_watcher/process/process.h"

#include "shared/errors/error.h"
#include "shared/system/win32/hwnd_utils.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <errhandlingapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <VersionHelpers.h>
#endif

#include <iostream>

namespace fs = std::filesystem;

namespace {

#ifdef _WIN32

std::unique_ptr<process_watcher::process::Process> createProcess(const PROCESSENTRY32& pe32) {
    try {
        // We only want to use windows that have a visible window.
        const HWND window = shared::system::win32::findWindowForProcessWithMaxDelay(pe32.th32ProcessID, std::chrono::milliseconds(1), std::chrono::milliseconds(0), true);
        if (window == NULL) {
            LOG_DEBUG("Failed to find window for process: " << " [" << pe32.th32ProcessID << "]: " << shared::errors::getWin32ErrorAsString() << std::endl);
            return nullptr;
        }
    } catch (std::exception& ex) {
        LOG_DEBUG("Failed to find window [exception] for process: " << ex.what() << std::endl);
        return nullptr;
    }

    // Need to take a snapshot to get the first module to get the full EXE path.
    HANDLE snapshot = INVALID_HANDLE_VALUE;
    while (snapshot == INVALID_HANDLE_VALUE) {
        snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pe32.th32ProcessID);

        // According to the MSDN documentatoin, returning ERROR_BAD_LENGTH means we should retry until
        // the function succeeds.
        if (snapshot == INVALID_HANDLE_VALUE) {
            const auto err = GetLastError();
            if (err == ERROR_BAD_LENGTH) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else {
                LOG_DEBUG("Failed to grab process snapshot: " << err << std::endl);
                return nullptr;
            }
        }
    }

    MODULEENTRY32W moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32W );
    if (!Module32FirstW(snapshot, &moduleEntry)) {
        LOG_DEBUG("Failed to get process first module: " << shared::errors::getWin32ErrorAsString() << std::endl);
        CloseHandle(snapshot);
        return nullptr;
    }

    // So there's a possibility that the casing of the module isn't the same as the process EXE.
    // So what we want to do is to combine the parent directory of the module with the EXE path of the original process.
    const auto path = fs::path(moduleEntry.szExePath).parent_path() / fs::path(pe32.szExeFile);
    return std::make_unique<process_watcher::process::Process>(path.native(), pe32.th32ProcessID);
}

#endif

}

namespace process_watcher::process {

bool listRunningProcesses(std::vector<Process>& out) {
#ifdef _WIN32
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (!snapshot) {
        LOG_WARNING("Failed to create process snapshot: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &pe32)) {
        LOG_WARNING("Failed to get first process: " << shared::errors::getWin32ErrorAsString() << std::endl);
        CloseHandle(snapshot);
        return false;
    }

    do {
        if (pe32.th32ProcessID == 0) {
            continue;
        }

        auto ptr = createProcess(pe32);
        if (!ptr) {
            continue;
        }
        out.emplace_back(*ptr);
    } while (Process32Next(snapshot, &pe32));
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

    CloseHandle(snapshot);
    return true;
}

}