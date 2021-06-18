#pragma once

#include <filesystem>
#include <string>
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
    Process(const std::string& path, OSProcessHandle pid): _path(path), _pid(pid) {}

#ifdef _WIN32
    Process(const std::wstring& path, OSProcessHandle pid): _path(path), _pid(pid) {}

    std::wstring name() const { return _path.filename().wstring(); }
#else
    std::string name() const { return _path.filename().string(); }
#endif
    const std::filesystem::path& path() const { return _path; }

    OSProcessHandle pid() const { return _pid; }
    bool empty() const { return _empty; }

private:
    std::filesystem::path _path;
    OSProcessHandle _pid;
    bool _empty = false;
};

bool listRunningProcesses(std::vector<Process>& out);

}