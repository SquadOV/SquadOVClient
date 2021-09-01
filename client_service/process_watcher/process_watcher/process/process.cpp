#include "process_watcher/process/process.h"

#include "shared/errors/error.h"
#include "shared/system/win32/hwnd_utils.h"

#include <algorithm>
#include <unordered_set>
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

std::wstring getProcessName(DWORD id) {
    // WE MUST USE PROCESS_QUERY_INFORMATION | PROCESS_VM_READ HERE.
    // PROCESS_QUERY_INFORMATION and PROCESS_QUERY_LIMITED_INFORMATION by themselves do not work on Windows 7.
    // Not that it really matters since Windows 7 not supported by Microsoft anymore but we had a couple of 
    // users who use it so...can't be picky about our users.
    const DWORD access = IsWindows8Point1OrGreater() ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION;
    HANDLE hProcess = OpenProcess(access, FALSE, id);

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

process_watcher::process::ProcessPtr createProcess(DWORD id) {
    auto p = std::make_shared<process_watcher::process::Process>(L"", id);
    p->updateName();
    return p;
}

#endif

}

namespace process_watcher::process {

void Process::updateName() {
    if (!empty()) {
        return;
    }

    // We want to update the name here if it wasn't previously set.
    _path = getProcessName(pid());
    _empty = _path.empty();
}

bool ProcessRunningState::update() {
#ifdef _WIN32
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded) ) {
        LOG_WARNING("Failed to enum processes: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return false;
    }

    cProcesses = cbNeeded / sizeof(DWORD);
    LOG_DEBUG("Detected " << cProcesses << " Processes using " << cbNeeded << " bytes out of " << sizeof(aProcesses) << std::endl);

    std::unordered_set<DWORD> currentProcessSet;
    for (DWORD i = 0; i < cProcesses; ++i) {
        currentProcessSet.insert(aProcesses[i]);
    }

    // First, go through the processes we have remembered as "running" and remove them if they are no longer running.
    std::vector<OSProcessHandle> pidToRemove;
    for (const auto& [pid, p]: _pidToProcess) {
        if (currentProcessSet.find(pid) != currentProcessSet.end()) {
            continue;
        }

        pidToRemove.push_back(pid);
    }

    for (const auto& pid: pidToRemove) {
        removeProcess(pid);
    }

    // Next, go through the current process list and create a new process for everything that we ddon't have tracked.
    for (DWORD i = 0; i < cProcesses; ++i) {
        if (aProcesses[i] == 0) {
            continue;
        }

        // No need to create the process if it already exists.
        if (getProcesssRunningByPid(aProcesses[i], false)) {
            continue;
        }

        addProcess(aProcesses[i]);
    }
#else
    THROW_ERROR("Unsupported OS for listing processes.");
#endif
    return true;
}

void ProcessRunningState::addProcess(OSProcessHandle pid) {
    auto p = createProcess(pid);
    if (!p) {
        THROW_ERROR("Failed to create process.");
        return;
    }

    if (!p->empty()) {
        _nameToProcess[p->name()] = p.get();
    }

    _pidToProcess[pid] = p;
}

void ProcessRunningState::removeProcess(OSProcessHandle pid) {
    auto it = _pidToProcess.find(pid);
    if (it == _pidToProcess.end()) {
        return;
    }

    if (!it->second->empty()) {
        _nameToProcess.erase(it->second->name());
    }

    _pidToProcess.erase(it);
}

std::optional<Process> ProcessRunningState::checkIfProcessCanBeUsed(const Process* p, bool needWindow) const {
    if (!p) {
        return std::nullopt;
    }

    if (needWindow) {
        try {
            // We only want to use windows that have a visible window.
            const HWND window = shared::system::win32::findWindowForProcessWithMaxDelay(p->pid(), std::chrono::milliseconds(1), std::chrono::milliseconds(0), true, false);
            if (window == NULL) {
                return std::nullopt;
            }
        } catch (std::exception& ex) {
            LOG_DEBUG("Failed to find window [exception] for process: " << ex.what() << std::endl);
            return std::nullopt;
        }
    }
    return *p;
}

std::optional<Process> ProcessRunningState::getProcesssRunningByName(const std::wstring& name, bool needWindow) const {
    const auto it = _nameToProcess.find(name);
    if (it == _nameToProcess.end()) {
        return std::nullopt;
    } else {
        return checkIfProcessCanBeUsed(it->second, needWindow);
    }
}

std::optional<Process> ProcessRunningState::getProcesssRunningByPid(OSProcessHandle pid, bool needWindow) const {
    const auto it = _pidToProcess.find(pid);
    if (it == _pidToProcess.end()) {
        return std::nullopt;
    } else {
        return checkIfProcessCanBeUsed(it->second.get(), needWindow);
    }
}

}