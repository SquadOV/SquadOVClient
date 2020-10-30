#pragma once

#include "process_watcher/process/process.h"
#include "shared/errors/error.h"

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#endif

#include <vector>

namespace process_watcher::memory {

// Read in a module's memory for a given process and performs RAII upon that data. 
// For now, we'll just want the module that has the same name as the process (the "main" module).
class ModuleMemoryMapper {
public:
    ModuleMemoryMapper(const process_watcher::process::Process& process, const std::string& moduleName);
    ~ModuleMemoryMapper();

    const std::vector<char>& moduleBuffer() const { return _buffer; }

    void readProcessMemory(std::vector<char>& buffer, uintptr_t src, size_t numBytes) const;
    void readProcessMemory(std::string& buffer, uintptr_t src, bool isCPtr) const;

    template<typename T, typename D>
    void readProcessMemory(T* buffer, D src) const {
        if (!ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(src), buffer, sizeof(T), nullptr)) {
            THROW_WIN32_ERROR("Failed to read process memory.");
        }
    }
private:
    // It's feasible that we'd want to read the memory multiple times or from different locations.
    // The default is to just read the module's data.
    HANDLE _hProcess;
    HMODULE _hMod;
    MODULEINFO _modInfo;
    std::vector<char> _buffer;
};

}