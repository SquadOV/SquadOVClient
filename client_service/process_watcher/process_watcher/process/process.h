#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace process_watcher::process {

#ifdef _WIN32
using OSProcessHandle = DWORD;
#endif

class Process {
public:
    Process(): _empty(true) {}
    Process(const std::string& path, OSProcessHandle pid): _path(path), _pid(pid), _empty(path.empty()) {}

#ifdef _WIN32
    Process(const std::wstring& path, OSProcessHandle pid): _path(path), _pid(pid), _empty(path.empty()) {}

    std::wstring name() const { return _path.filename().wstring(); }
#else
    std::string name() const { return _path.filename().string(); }
#endif
    const std::filesystem::path& path() const { return _path; }

    OSProcessHandle pid() const { return _pid; }
    bool empty() const { return _empty; }

    void updateName();
private:
    std::filesystem::path _path;
    OSProcessHandle _pid;
    bool _empty = false;
};

using ProcessPtr = std::shared_ptr<Process>;

class ProcessRunningState {
public:
    bool update();

#ifdef _WIN32
    std::optional<Process> getProcesssRunningByName(const std::wstring& name, bool needWindow) const;
#endif

    std::optional<Process> getProcesssRunningByPid(OSProcessHandle pid, bool needWindow) const;

private:
    // Certain processes we don't want to actually return as "running" until certain conditions are met (i.e. it has an active window).
    std::optional<Process> checkIfProcessCanBeUsed(const Process* p, bool needWindow) const;

    void addProcess(OSProcessHandle pid);
    void removeProcess(OSProcessHandle pid);

#ifdef _WIN32
    std::unordered_map<std::wstring, Process*> _nameToProcess;
#endif

    std::unordered_map<OSProcessHandle, ProcessPtr> _pidToProcess;
};

}