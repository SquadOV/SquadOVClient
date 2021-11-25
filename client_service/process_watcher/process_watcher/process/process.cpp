#include "process_watcher/process/process.h"

#include "shared/errors/error.h"
#include "shared/system/win32/hwnd_utils.h"
#include "shared/system/interfaces/system_process_interface.h"
#include "shared/strings/strings.h"
#include "shared/filesystem/utility.h"

#include <algorithm>
#include <unordered_set>
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <errhandlingapi.h>
#include <winternl.h>
#endif

#include <iostream>

namespace {

#ifdef _WIN32

process_watcher::process::ProcessPtr createProcess(const shared::system::SystemProcessInterfacePtr& itf, DWORD id) {
    auto p = std::make_shared<process_watcher::process::Process>(itf, L"", id);
    p->updateName();
    return p;
}

#endif

}

namespace process_watcher::process {

    
ProcessRecord ProcessRecord::fromJson(const nlohmann::json& obj) {
    ProcessRecord record;
    record.name = obj.value("name", "");
    record.exe = obj.value("exe", "");
    record.ico = obj.value("ico", "");
    return record;
}

void Process::updateName() {
    if (!empty()) {
        return;
    }

    // We want to update the name here if it wasn't previously set.
    _path = _itf->getProcessName(pid());
    _empty = _path.empty();
}

ProcessRunningState::ProcessRunningState(const shared::system::SystemProcessInterfacePtr& itf):
    _itf(itf)
{
}

bool ProcessRunningState::update() {
#ifdef _WIN32
    const auto processes = _itf->enumProcesses();
    LOG_DEBUG("Detected " << processes.size() << " Processes" << std::endl);

    clearProcesses();
    // Go through the current process list and create a new process for everything that we don't have tracked.
    for (const auto pid: processes) {
        addProcess(pid);
    }
#else
    THROW_ERROR("Unsupported OS for listing processes.");
#endif
    return true;
}

void ProcessRunningState::addProcess(OSPID pid) {
    auto p = createProcess(_itf, pid);
    if (!p) {
        THROW_ERROR("Failed to create process.");
        return;
    }

    if (!p->empty()) {
        _nameToProcess[p->name()].push_back(p.get());
    }

    _pidToProcess[pid] = p;
}

void ProcessRunningState::clearProcesses() {
    _nameToProcess.clear();
    _pidToProcess.clear();
}

std::optional<Process> ProcessRunningState::checkIfProcessCanBeUsed(const Process* p, bool needWindow) const {
    if (!p) {
        return std::nullopt;
    }

    if (needWindow) {
        try {
            // We only want to use windows that have a visible window.
            const HWND window = _itf->findWindowForProcessWithMaxDelay(p->pid(), std::chrono::milliseconds(1), std::chrono::milliseconds(0), true, false);
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

std::vector<Process> ProcessRunningState::getProcesssRunningByName(const std::wstring& name, bool needWindow) const {
    const auto it = _nameToProcess.find(name);

    std::vector<Process> ret;

    if (it != _nameToProcess.end()) {
        for (const auto& p: it->second) {
            auto pp = checkIfProcessCanBeUsed(p, needWindow);
            if (!pp) {
                continue;
            }
            ret.push_back(pp.value());
        }
    }

    return ret;
}

std::optional<Process> ProcessRunningState::getProcesssRunningByPid(OSPID pid, bool needWindow) const {
    const auto it = _pidToProcess.find(pid);
    if (it == _pidToProcess.end()) {
        return std::nullopt;
    } else {
        return checkIfProcessCanBeUsed(it->second.get(), needWindow);
    }
}

std::vector<ProcessRecord> ProcessRunningState::getList() const {
    std::vector<ProcessRecord> ret;
    for (const auto& p: _pidToProcess) {
        ProcessRecord rec;
        rec.pid = p.second->pid();
        rec.fullPath = shared::filesystem::pathUtf8(p.second->path());
        rec.exe = shared::strings::wcsToUtf8(p.second->name());
        try {
            rec.name = shared::strings::wcsToUtf8(_itf->getProcessFriendlyName(p.second->path()));
        } catch (...) {
            rec.name = "";
        }
        
        if (rec.name.empty()) {
            rec.name = rec.exe;
        }

        rec.ico = "";
        ret.push_back(rec);
    }
    return ret;
}

}