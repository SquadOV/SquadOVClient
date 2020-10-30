#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include <iostream>
#include <unordered_map>

namespace process_watcher::memory {

enum class ExecutableArch {
    X86,
    X86_64
};


// Extracts information about the portable executable (PE) from the loaded module's memory.
// Informationa bout the PE format can be found here: https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
class PEMapper {
public:
    explicit PEMapper(const ModuleMemoryMapperSPtr& mapper);

    uint32_t getExportRva(const std::string& name) const { return _exportRvas.at(name); }
    ExecutableArch arch() const;

    friend std::ostream& operator<<(std::ostream& os, const PEMapper& map);
private:
    const char* loadMsDosStub(const char* buffer);
    const char* loadPEHeader(const char* buffer);
    const char* loadExports(const char* buffer);

    std::string loadExportName(const char* base, const char* nameTable, uint32_t index) const;
    uint16_t loadOrdinal(const char* ordinalTable, uint32_t index) const;
    uint32_t loadAddressRvaFromOrdinal(const char* base, const char* addressTable, uint16_t ordinal) const;

    struct MSDosStub {
        uint16_t e_magic;
        uint32_t e_lfanew;
    };
    MSDosStub _msdos;

    struct COFFHeader {
        uint16_t machine;
        uint16_t numSections;
        uint32_t timeDateStamp;
        uint32_t pptrToSymbolTable;
        uint32_t numberOfSymbols;
        uint16_t sizeOfOptionalHeader;
        uint16_t characteristics;
    };

    struct PEDataDirectory {
        uint32_t rva;
        uint32_t size;
    };

    struct PEOptionalHeader {
        uint16_t peMagic;
        uint32_t numRva;
        PEDataDirectory exportTableRva;
    };

    struct ExportDirectoryTable {
        uint32_t exportFlags;
        uint32_t timeDateStamp;
        uint16_t majorVersion;
        uint16_t minorVersion;
        uint32_t nameRva;
        uint32_t ordinalBase;
        uint32_t addressTableEntries;
        uint32_t numNamePointers;
        uint32_t exportAddressTableRva;
        uint32_t namePointerRva;
        uint32_t ordinalTableRva;
    };

    struct PEFile  {
        uint32_t peSignature;
        COFFHeader coff;
        PEOptionalHeader optional;
        ExportDirectoryTable exports;
    };

    PEFile _file;
    std::unordered_map<std::string, uint32_t> _exportRvas;
};

std::ostream& operator<<(std::ostream& os, const PEMapper& map);

}