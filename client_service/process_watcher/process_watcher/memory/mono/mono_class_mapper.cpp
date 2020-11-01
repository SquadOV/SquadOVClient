#include "process_watcher/memory/mono/mono_class_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

#include <sstream>

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONO_CLASS_FLAGS_OFFSET = 20;
constexpr uint8_t MONO_CLASS_VALUETYPE_FLAGS_BITFLAGS = 0b100;

constexpr uint32_t MONO_CLASS_KIND_OFFSET = 30;
constexpr uint32_t MONO_CLASS_NAME_OFFSET = 44;
constexpr uint32_t MONO_CLASS_NAMESPACE_OFFSET = 48;
constexpr uint32_t MONO_CLASS_VTABLE_SIZE_OFFSET = 56;
constexpr uint32_t MONO_CLASS_SIZES_OFFSET = 92;
constexpr uint32_t MONO_CLASS_CLASSFIELDS_OFFSET = 96;
constexpr uint32_t MONO_CLASS_RUNTIME_INFO_OFFSET = 132;
constexpr uint32_t MONO_CLASS_SIZE = 148; // I'm assuming I counted MonoType wrong or something.
constexpr uint32_t MONO_CLASSDEF_FIELD_COUNT_OFFSET = MONO_CLASS_SIZE + 16;
constexpr uint32_t MONO_CLASSGENERICINST_GENERIC_CLASS_OFFSET = MONO_CLASS_SIZE;
constexpr uint32_t MONO_GENERICCLASS_CONTAINER_CLASS_OFFSET = 0;
constexpr uint32_t MONO_CLASSFIELDS_SIZE = 16;
constexpr uint32_t MONO_RUNTIME_INFO_VTABLES_OFFSET = 4;

enum class MonoClassKind {
    MONO_CLASS_DEF = 1,
    MONO_CLASS_GTD,
    MONO_CLASS_GINST,
    MONO_CLASS_GPARAM,
    MONO_CLASS_ARRAY,
    MONO_CLASS_POINTER
};
uint32_t loadFieldCount(const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr);
uint32_t loadClassDefFieldCount(const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr) {
    uint32_t ret = 0;
    memory->readProcessMemory(&ret, ptr + MONO_CLASSDEF_FIELD_COUNT_OFFSET);
    return ret;
}

uint32_t loadGenericClassFieldCount(const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr) {
    uint32_t genericClassPtr = 0;
    memory->readProcessMemory(&genericClassPtr, ptr + MONO_CLASSGENERICINST_GENERIC_CLASS_OFFSET);

    uint32_t containerClassPtr = 0;
    memory->readProcessMemory(&containerClassPtr, static_cast<uintptr_t>(genericClassPtr + MONO_GENERICCLASS_CONTAINER_CLASS_OFFSET));

    return loadFieldCount(memory, static_cast<uintptr_t>(containerClassPtr));
}

uint32_t loadFieldCount(const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr) {
    uint8_t rawClassKind = 0;
    memory->readProcessMemory(&rawClassKind, ptr + MONO_CLASS_KIND_OFFSET);

    // class_kind is a 3-bit field in the most significant bits.
    // We're assuming *little endian* here so it should occupy the
    // last 3 bits in memory.
    rawClassKind &= 0b111;

    switch (static_cast<MonoClassKind>(rawClassKind)) {
        case MonoClassKind::MONO_CLASS_DEF:
        case MonoClassKind::MONO_CLASS_GTD:
            return loadClassDefFieldCount(memory, ptr);
        case MonoClassKind::MONO_CLASS_GINST:
            return loadGenericClassFieldCount(memory, ptr);
        case MonoClassKind::MONO_CLASS_GPARAM:
        case MonoClassKind::MONO_CLASS_ARRAY:
        case MonoClassKind::MONO_CLASS_POINTER:
            return 0;
    }

    return 0;
}

}

MonoClassMapper::MonoClassMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr):
    _image(image),
    _memory(memory),
    _ptr(ptr) {
}

void MonoClassMapper::loadInner() {
    _memory->readProcessMemory(_name, _ptr + MONO_CLASS_NAME_OFFSET, true);
    _memory->readProcessMemory(_namespace, _ptr + MONO_CLASS_NAMESPACE_OFFSET, true);

    // Read all the class fields. This is just a C array of objects of the type MonoClassField.
    // Getting the *size* of this array is a little tricky and depends on the class_kind.
    const auto numFields = loadFieldCount(_memory, _ptr);
    uint32_t classFieldsPtr = 0;
    _memory->readProcessMemory(&classFieldsPtr, _ptr + MONO_CLASS_CLASSFIELDS_OFFSET);

    for (uint32_t i = 0; i < numFields && classFieldsPtr; ++i) {
        auto classfield = std::make_unique<MonoClassFieldMapper>(_image, _memory, classFieldsPtr);
        const auto nm = classfield->name();
        _fields[nm] = std::move(classfield);
        classFieldsPtr += MONO_CLASSFIELDS_SIZE;
    }

    _sizes = _memory->readProcessMemory<int32_t>(_ptr + MONO_CLASS_SIZES_OFFSET);

    const auto flags = _memory->readProcessMemory<uint8_t>(_ptr + MONO_CLASS_FLAGS_OFFSET);
    _isValueType = flags & MONO_CLASS_VALUETYPE_FLAGS_BITFLAGS;
}

std::string MonoClassMapper::fullName() const {
    std::ostringstream nm;
    if (!_namespace.empty()) {
        nm << _namespace << ".";
    }
    nm << _name;
    return nm.str();
}

const MonoVTableMapper* MonoClassMapper::loadVTable(int32_t domainId) {
    auto it = _domainVtables.find(domainId);
    if (it != _domainVtables.end()) {
        return (*it).second.get();
    }

    int32_t vtableSize = 0;
    _memory->readProcessMemory(&vtableSize, _ptr + MONO_CLASS_VTABLE_SIZE_OFFSET);

    // Grab the runtime_info to get the MonoClassRuntimeInfo object.
    // Ensure the input domain ID is less than or equal to max_domain (array size is max_domain + 1) and
    // then use the MonoVTable pointer to create a MonoVTableMapper.
    uint32_t runtimeInfoPtr = 0;
    _memory->readProcessMemory(&runtimeInfoPtr, _ptr + MONO_CLASS_RUNTIME_INFO_OFFSET);

    uint16_t maxDomains = 0;
    _memory->readProcessMemory(&maxDomains, static_cast<uintptr_t>(runtimeInfoPtr));

    if (domainId < 0 || domainId >= static_cast<int32_t>(maxDomains)) {
        THROW_ERROR("Invalid domain ID.");
    }
    
    uint32_t vtablePtr = 0;
    _memory->readProcessMemory(&vtablePtr, static_cast<uintptr_t>(runtimeInfoPtr + MONO_RUNTIME_INFO_VTABLES_OFFSET));

    auto vtable = std::make_unique<MonoVTableMapper>(_image, _memory, vtablePtr, vtableSize, domainId);
    auto* ptr = vtable.get();
    _domainVtables[domainId] = std::move(vtable);
    return ptr;
}

std::ostream& operator<<(std::ostream& os, const MonoClassMapper& map) {
    os << "Name: " << map._name << std::endl
        << "Namespace: " << map._namespace << std::endl
        << "Fields: ";

    for (const auto& [key, field] : map._fields) {
        os << std::endl << "--- " << key << " ---";
        os << std::endl << *field;
    }

    return os;
}

}