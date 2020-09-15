#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace process_watcher::process {

class Process {
public:
    Process(const std::string& path): _path(path) {}

#ifdef _WIN32
    std::wstring name() const { return _path.filename().wstring(); }
#else
    std::string name() const { return _path.filename().string(); }
#endif

private:
    std::filesystem::path _path;
};

bool listRunningProcesses(std::vector<Process>& out);

}