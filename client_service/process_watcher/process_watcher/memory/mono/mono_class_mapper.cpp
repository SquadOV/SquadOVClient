#include "process_watcher/memory/mono/mono_class_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

#include <sstream>

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONO_INSTANCE_SIZE_OFFSET = 16;
constexpr uint32_t MONO_CLASS_FLAGS_OFFSET = 20;
constexpr uint8_t MONO_CLASS_VALUETYPE_FLAGS_BITFLAGS = 0b100;
constexpr uint8_t MONO_CLASS_ENUMTYPE_FLAGS_BITFLAGS = 0b1000;

constexpr uint32_t MONO_CLASS_ELEMENTCLASS_OFFSET = 0;
constexpr uint32_t MONO_CLASS_SUPERTYPES_OFFSET = 8;
constexpr uint32_t MONO_CLASS_IDEPTH_OFFSET = 12;
constexpr uint32_t MONO_CLASS_KIND_OFFSET = 30;
constexpr uint32_t MONO_CLASS_NAME_OFFSET = 44;
constexpr uint32_t MONO_CLASS_NAMESPACE_OFFSET = 48;
constexpr uint32_t MONO_CLASS_VTABLE_SIZE_OFFSET = 56;
constexpr uint32_t MONO_CLASS_SIZES_OFFSET = 92;
constexpr uint32_t MONO_CLASS_CLASSFIELDS_OFFSET = 96;
constexpr uint32_t MONO_CLASS_THISARG_OFFSET = 104;
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
    const auto eleClassPtr = _memory->readProcessMemory<uint32_t>(_ptr + MONO_CLASS_ELEMENTCLASS_OFFSET);
    _elementClass = _image->loadClassFromPtr(static_cast<uintptr_t>(eleClassPtr));

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

    _type = std::make_unique<MonoTypeMapper>(_image, _memory, _ptr + MONO_CLASS_THISARG_OFFSET);
    _type->loadInner();

    _sizes = _memory->readProcessMemory<int32_t>(_ptr + MONO_CLASS_SIZES_OFFSET);

    const auto flags = _memory->readProcessMemory<uint8_t>(_ptr + MONO_CLASS_FLAGS_OFFSET);
    _isValueType = flags & MONO_CLASS_VALUETYPE_FLAGS_BITFLAGS;
    _isEnumType = flags & MONO_CLASS_ENUMTYPE_FLAGS_BITFLAGS;

    _instanceSize = _memory->readProcessMemory<int32_t>(_ptr + MONO_INSTANCE_SIZE_OFFSET);

    const uint32_t supertypesArrayPtr = _memory->readProcessMemory<uint32_t>(_ptr + MONO_CLASS_SUPERTYPES_OFFSET);
    if (supertypesArrayPtr) {
        
        const auto idepth = _memory->readProcessMemory<uint16_t>(_ptr + MONO_CLASS_IDEPTH_OFFSET);
        for (uint16_t i = 0; i < idepth; ++i) {
            const uint32_t supertypesPtr = _memory->readProcessMemory<uint32_t>(supertypesArrayPtr + i  * 4);
            const auto* superklass = _image->loadClassFromPtr(static_cast<uintptr_t>(supertypesPtr));
            _supertypes[superklass->name()] = superklass;
        }
    }
}

std::string MonoClassMapper::fullName() const {
    std::ostringstream nm;
    if (!_namespace.empty()) {
        nm << _namespace << ".";
    }
    nm << _name;
    return nm.str();
}

const MonoClassFieldMapper* MonoClassMapper::field(const std::string& nm) const {
    const auto it = _fields.find(nm);
    if (it == _fields.end()) {
        LOG_WARNING("Failed to find field: " << nm << " on klass " << fullName() << std::endl);
        return nullptr;
    } else {
        return it->second.get();
    }
}

const MonoVTableMapper* MonoClassMapper::loadVTable(int32_t domainId) {
    auto it = _domainVtables.find(domainId);
    if (it != _domainVtables.end()) {
        return (*it).second.get();
    }

    int32_t vtableSize = 0;
    _memory->readProcessMemory(&vtableSize, _ptr + MONO_CLASS_VTABLE_SIZE_OFFSET);

    if (!vtableSize) {
        return nullptr;
    }

    // Grab the runtime_info to get the MonoClassRuntimeInfo object.
    // Ensure the input domain ID is less than or equal to max_domain (array size is max_domain + 1) and
    // then use the MonoVTable pointer to create a MonoVTableMapper.
    uint32_t runtimeInfoPtr = 0;
    _memory->readProcessMemory(&runtimeInfoPtr, _ptr + MONO_CLASS_RUNTIME_INFO_OFFSET);

    if (!runtimeInfoPtr) {
        return nullptr;
    }

    uint16_t maxDomains = 0;
    _memory->readProcessMemory(&maxDomains, static_cast<uintptr_t>(runtimeInfoPtr));

    if (domainId < 0 || domainId >= static_cast<int32_t>(maxDomains)) {
        return nullptr;
    }
    
    uint32_t vtablePtr = 0;
    _memory->readProcessMemory(&vtablePtr, static_cast<uintptr_t>(runtimeInfoPtr + MONO_RUNTIME_INFO_VTABLES_OFFSET));

    auto vtable = std::make_unique<MonoVTableMapper>(_image, _memory, vtablePtr, vtableSize, domainId);
    auto* ptr = vtable.get();
    _domainVtables[domainId] = std::move(vtable);
    return ptr;
}


int32_t MonoClassMapper::instanceSize() const {
    return _instanceSize;
}

int32_t MonoClassMapper::valueSize() const {
    return instanceSize() - 8;
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