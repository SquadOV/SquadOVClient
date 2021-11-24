#pragma once

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "shared/system/interfaces/system_process_interface.h"
#include "shared/json.h"

namespace process_watcher::process {

struct ProcessRecord {
    // name: User friendly name to present to the user.
    std::string name;
    // The basename of the exe file that we can identify the exe later.
    // Also what we use the key to cache the icon.
    std::string exe;
    // base64 icon
    std::string ico;
    // C++ internals - never serialized.
    std::string fullPath;
    OSPID pid;

    static ProcessRecord fromJson(const nlohmann::json& obj);
};

class Process {
public:
    Process(): _empty(true) {}
    Process(const shared::system::SystemProcessInterfacePtr& itf, const std::string& path, OSPID pid): _itf(itf), _path(path), _pid(pid), _empty(path.empty()) {}

#ifdef _WIN32
    Process(const shared::system::SystemProcessInterfacePtr& itf, const std::wstring& path, OSPID pid): _itf(itf), _path(path), _pid(pid), _empty(path.empty()) {}

    std::wstring name() const { return _path.filename().wstring(); }
#else
    std::string name() const { return _path.filename().string(); }
#endif
    const std::filesystem::path& path() const { return _path; }

    OSPID pid() const { return _pid; }
    bool empty() const { return _empty; }

    void updateName();
private:
    shared::system::SystemProcessInterfacePtr _itf;
    std::filesystem::path _path;
    OSPID _pid;
    bool _empty = false;
};

using ProcessPtr = std::shared_ptr<Process>;

class ProcessRunningState {
public:
    explicit ProcessRunningState(const shared::system::SystemProcessInterfacePtr& itf);
    bool update();

#ifdef _WIN32
    std::optional<Process> getProcesssRunningByName(const std::wstring& name, bool needWindow) const;
#endif

    std::optional<Process> getProcesssRunningByPid(OSPID pid, bool needWindow) const;
    std::vector<ProcessRecord> getList() const;

private:
    // Certain processes we don't want to actually return as "running" until certain conditions are met (i.e. it has an active window).
    std::optional<Process> checkIfProcessCanBeUsed(const Process* p, bool needWindow) const;

    void addProcess(OSPID pid);
    void clearProcesses();

#ifdef _WIN32
    std::unordered_map<std::wstring, Process*> _nameToProcess;
#endif

    std::unordered_map<OSPID, ProcessPtr> _pidToProcess;
    shared::system::SystemProcessInterfacePtr _itf;
};

}

namespace shared::json {

template<>
struct JsonConverter<process_watcher::process::ProcessRecord> {
    static nlohmann::json to(const process_watcher::process::ProcessRecord& v) {
        nlohmann::json ret = {
            { "name" , v.name },
            { "exe" , v.exe },
            { "ico" , v.ico }
        };
        return ret;
    }
};

}