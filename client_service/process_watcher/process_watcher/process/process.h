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
    Process(const std::string& path, OSProcessHandle pid): _path(path), _pid(pid) {}

#ifdef _WIN32
    std::wstring name() const { return _path.filename().wstring(); }
#else
    std::string name() const { return _path.filename().string(); }
#endif

    OSProcessHandle pid() const { return _pid; }

private:
    std::filesystem::path _path;
    OSProcessHandle _pid;
};

bool listRunningProcesses(std::vector<Process>& out);

}