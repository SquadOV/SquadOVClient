#include "process_watcher/process/process.h"

#include <algorithm>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <errhandlingapi.h>
#endif

#include <iostream>

namespace {

#ifdef _WIN32

std::unique_ptr<process_watcher::process::Process> createProcess(DWORD id) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, id);

    if (hProcess == NULL) {
        return nullptr;
    }

    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    if (GetModuleFileNameEx(hProcess, NULL, szProcessName, sizeof(szProcessName)/sizeof(TCHAR)) == 0) {
        return nullptr;
    }
    CloseHandle(hProcess);

    return std::make_unique<process_watcher::process::Process>(std::string(szProcessName));
}

#endif

}

namespace process_watcher::process {

bool listRunningProcesses(std::vector<Process>& out) {
#ifdef _WIN32
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded) ) {
        return false;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

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
    throw std::runtime_error("Unsupported OS for listing processes.");
#endif

    std::sort(out.begin(), out.end(), [](const Process& a, const Process& b) {
        return a.name() < b.name();
    });

    return true;
}

}