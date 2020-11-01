#include "process_watcher/memory/module_memory_mapper.h"


#ifdef _WIN32
#include <TlHelp32.h>
#endif

#include <chrono>
#include <thread>

namespace process_watcher::memory {

ModuleMemoryMapper::ModuleMemoryMapper(const process_watcher::process::Process& process, const std::string& moduleName) {
    _hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process.pid());
    if (!_hProcess) {
        THROW_WIN32_ERROR("Failed to open process for module memory mapping");
        return;
    }

    // Need to get a handle to the module that we care about.
    // We need to use CreateToolhelp32Snapshot here instead of EnumProcessModules as
    // EnumProcessModules won't display some DLLs (namely the DLLs loaded with the LOAD_LIBRARY_AS_DATAFILE flag according to
    // MSDN). This will include DLLs such as mono.
    HANDLE snapshot = INVALID_HANDLE_VALUE;
    while (snapshot == INVALID_HANDLE_VALUE) {
        snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process.pid());

        // According to the MSDN documentatoin, returning ERROR_BAD_LENGTH means we should retry until
        // the function succeeds.
        if (snapshot == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_BAD_LENGTH) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else {
                THROW_WIN32_ERROR("Failed to grab process snapshot.");
            }
        }
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(snapshot, &moduleEntry)) {
        CloseHandle(snapshot);
        THROW_WIN32_ERROR("Failed to grab snapshot module.");
    }

    do {
        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
        if (!GetModuleBaseNameA(_hProcess, moduleEntry.hModule, szProcessName, sizeof(TCHAR)*MAX_PATH)) {
            continue;
        }
        
        if (std::string(szProcessName) == moduleName) {
            _hMod = moduleEntry.hModule;
            break;
        }
    } while (Module32Next(snapshot, &moduleEntry));
    CloseHandle(snapshot);

    // Pull information in about the module so that we can grab the appropriate location
    // to start reading memory from.
    if (!GetModuleInformation(_hProcess, _hMod, &_modInfo, sizeof(MODULEINFO))) {
        THROW_WIN32_ERROR("Failed obtain module information");
        return;
    }

    readProcessMemory(_buffer, reinterpret_cast<uintptr_t>(_modInfo.lpBaseOfDll), _modInfo.SizeOfImage);
}

ModuleMemoryMapper::~ModuleMemoryMapper() {
    CloseHandle(_hProcess);
}

// This function is needed because we can't just ReadProcessMemory straight into
// the buffer. We need to handle the case of ERROR_PARTIAL_COPY properly so that we
// read all the memory we want.
void ModuleMemoryMapper::readProcessMemory(std::vector<char>& buffer, uintptr_t src, size_t numBytes) const {
    buffer.resize(numBytes);
    if (!ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(src), buffer.data(), numBytes, nullptr)) {
        THROW_WIN32_ERROR("Failed to read process memory.");
    }
}

void ModuleMemoryMapper::readProcessMemory(std::string& buffer, uintptr_t src, size_t length) const {
    buffer.resize(length);
    if (!ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(src), buffer.data(), length, nullptr)) {
        THROW_WIN32_ERROR("Failed to read process memory.");
    }
}

void ModuleMemoryMapper::readProcessMemory(std::u16string& buffer, uintptr_t src, size_t length) const {
    buffer.resize(length);
    if (!ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(src), buffer.data(), length * 2, nullptr)) {
        THROW_WIN32_ERROR("Failed to read process memory.");
    }
}

void ModuleMemoryMapper::readProcessMemory(std::string& buffer, uintptr_t src, bool isCPtr) const {
    if (isCPtr) {
        // In this case the 'src' is a 'const char*' so we actually need to do another redirect to read the C string.
        uint32_t redirect = 0;
        readProcessMemory(&redirect, src);

        if (redirect) {
            readProcessMemory(buffer, static_cast<uintptr_t>(redirect), false);
        }
        return;
    }

    constexpr size_t stepSize = 128;
    buffer.clear();

    bool foundNull = false;
    char tmpBuffer[stepSize];
    while (!foundNull) {
        if (!ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(src + buffer.size()), tmpBuffer, stepSize, nullptr)) {
            THROW_WIN32_ERROR("Failed to read process memory.");
        }

        size_t numToCopy = 0;
        for (size_t i = 0; i < stepSize; ++i) {
            if (tmpBuffer[i] == '\0') {
                foundNull = true;
                break;
            }
            ++numToCopy;
        }

        const auto oldSize = buffer.size();
        buffer.resize(buffer.size() + numToCopy);
        std::memcpy(buffer.data() + oldSize, tmpBuffer, numToCopy);
    }
}

}