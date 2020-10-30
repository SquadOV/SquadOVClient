#include "process_watcher/memory/pe_mapper.h"
#include "shared/errors/error.h"

namespace process_watcher::memory {
namespace {

// Every PE file has the same magic number in the first 16 bits.
constexpr uint16_t MSDOS_MAGIC_NUMBER = 0x5A4D;
// This is the offset from the start of the file to where the pointer to the PE signature is (e_lfanew).
constexpr uint8_t MSDOS_PE_SIGNATURE_OFFSET = 0x3c;
constexpr uint32_t PE_SIGNATURE = 0x4550;

constexpr uint16_t PE_AMD64_MACHINE = 0x8664;
constexpr uint16_t PE_32BIT_MACHINE = 0x14c;

constexpr uint16_t PE_MAGIC_PE32 = 0x10b;
constexpr uint16_t PE_MAGIC_PE32_PLUS = 0x20b;

// Offsets taken from https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
constexpr uint32_t PE32_OPTIONAL_HEADER_DATA_DIR_OFFSET = 28 + 68;
constexpr uint32_t PE32_PLUS_OPTIONAL_HEADER_DATA_DIR_OFFSET = 24 + 88;
}

PEMapper::PEMapper(const ModuleMemoryMapper& mapper) {
    const auto& buffer = mapper.moduleBuffer();
    
    {
        const char* rawBuffer = buffer.data();
        rawBuffer = loadMsDosStub(rawBuffer);
        rawBuffer = loadPEHeader(rawBuffer);
    }

    loadExports(buffer.data());
}

const char* PEMapper::loadMsDosStub(const char* buffer) {
    // First make sure we're actually dealing with a PE. They must
    // start with the MSDOS_MAGIC_NUMBER.
    std::memcpy(&_msdos.e_magic, buffer, sizeof(_msdos.e_magic));
    if (_msdos.e_magic != MSDOS_MAGIC_NUMBER) {
        THROW_ERROR("Expected MS-DOS Magic number: " << std::hex << _msdos.e_magic << std::dec << std::endl);
    }

    // Read in e_lfanew which is the offset from the beginning of the file to the PE signature.
    std::memcpy(&_msdos.e_lfanew, buffer + MSDOS_PE_SIGNATURE_OFFSET, sizeof(_msdos.e_lfanew));
    buffer += _msdos.e_lfanew;
    return buffer;
}

const char* PEMapper::loadPEHeader(const char* buffer) {
    // Read and verify PE file signature
    std::memcpy(&_file.peSignature, buffer, sizeof(_file.peSignature));
    if (_file.peSignature != PE_SIGNATURE) {
        THROW_ERROR("Expected PE Magic number: " << std::hex << _file.peSignature << std::dec << std::endl);
    }
    buffer += sizeof(_file.peSignature);

    // Read the COFF -- ensure that the optional header exists and that the machine type is as expected.
    // Not sure if we'll realistically ever see the other ones but better safe than sorry.
    std::memcpy(&_file.coff, buffer, sizeof(COFFHeader));
    if (_file.coff.machine != PE_AMD64_MACHINE && _file.coff.machine != PE_32BIT_MACHINE) {
        THROW_ERROR("Unsupported machine type: " << std::hex << _file.coff.machine << std::dec << std::endl);
    }
    buffer += sizeof(COFFHeader);

    if (!_file.coff.sizeOfOptionalHeader) {
        THROW_ERROR("Empty COFF optional header.");
    }

    // Read in the PE magic number - this is what we'll use to offset the appropriate amount to the pointers to
    // the tables that we want to read from. 
    std::memcpy(&_file.optional.peMagic, buffer, sizeof(_file.optional.peMagic));
    if (_file.optional.peMagic == PE_MAGIC_PE32) {
        buffer += PE32_OPTIONAL_HEADER_DATA_DIR_OFFSET;
    } else if (_file.optional.peMagic == PE_MAGIC_PE32_PLUS) {
        buffer += PE32_PLUS_OPTIONAL_HEADER_DATA_DIR_OFFSET;
    } else {
        THROW_ERROR("Unsupported PE magic number: " << std::hex << _file.optional.peMagic << std::dec << std::endl);
    }

    // Read the number of RVA entries (number of data directories). We need to ensure that this is at least 1
    // to make sure that there exists at least the export table.
    std::memcpy(&_file.optional.numRva, buffer - 4, sizeof(_file.optional.numRva));
    if (!_file.optional.numRva) {
        THROW_ERROR("Zero RVA");
    }

    std::memcpy(&_file.optional.exportTableRva, buffer, sizeof(_file.optional.exportTableRva));
    buffer += sizeof(_file.optional.exportTableRva);
    return buffer;
}

const char* PEMapper::loadExports(const char* buffer) {
    std::memcpy(&_file.exports, buffer + _file.optional.exportTableRva.rva, sizeof(ExportDirectoryTable));

    // Iterate throug the number of name pointers instead of the number of
    // address table entries as there's a (documented) guarantee that the
    // number of name pointers is equal to the number of entries in the ordinal
    // table. Then we can use the ordinal table to index uinto the address table
    // to get the actual RVA.
    const char* addressTable = buffer + _file.exports.exportAddressTableRva;
    const char* nameTable = buffer + _file.exports.namePointerRva;
    const char* ordinalTable = buffer + _file.exports.ordinalTableRva;

    _exportRvas.reserve(_file.exports.numNamePointers);
    for (uint32_t i = 0; i < _file.exports.numNamePointers; ++i) {
        const auto name = loadExportName(buffer, nameTable, i);
        const auto ordinal = loadOrdinal(ordinalTable, i);
        const auto addr = loadAddressRvaFromOrdinal(buffer, addressTable, ordinal);
        _exportRvas[name] = addr;
    }

    return buffer;
}

std::string PEMapper::loadExportName(const char* base, const char* nameTable, uint32_t index) const {
    // Read the 32-bit pointer to the null-terminated string.
    uint32_t nameRva = 0;
    std::memcpy(&nameRva, nameTable + sizeof(nameRva) * index, sizeof(nameRva));
    return std::string(base + nameRva);
}

uint16_t PEMapper::loadOrdinal(const char* ordinalTable, uint32_t index) const {
    uint16_t ordinal = 0;
    std::memcpy(&ordinal, ordinalTable + index * sizeof(ordinal), sizeof(ordinal));
    return ordinal;
}

uint32_t PEMapper::loadAddressRvaFromOrdinal(const char* base, const char* addressTable, uint16_t ordinal) const {
    uint32_t rva = 0;
    std::memcpy(&rva, addressTable + ordinal * sizeof(rva), sizeof(rva));
    return rva;
}

ExecutableArch PEMapper::arch() const {
    if (_file.coff.machine == PE_32BIT_MACHINE) {
        return ExecutableArch::X86;
    } else { 
        return ExecutableArch::X86_64;
    }
}

}