#include "process_watcher/memory/mono/mono_type_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

#include <sstream>

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONOTYPE_DATA_OFFSET = 0;
constexpr uint32_t MONOTYPE_ATTRS_OFFSET = 4;
constexpr uint32_t MONOTYPE_TYPE_OFFSET = 6;

}

std::string monoTypeToString(MonoTypes typ) {
    switch (typ) {
        case MonoTypes::Void:
            return "Void";
        case MonoTypes::Bool:
            return "Bool";
        case MonoTypes::Char:
            return "Char";
        case MonoTypes::Int1:
            return "Int1";
        case MonoTypes::Uint1:
            return "Uint1";
        case MonoTypes::Int2:
            return "Int2";
        case MonoTypes::Uint2:
            return "Uint2";
        case MonoTypes::Int4:
            return "Int4";
        case MonoTypes::Uint4:
            return "Uint4";
        case MonoTypes::Int8:
            return "Int8";
        case MonoTypes::Uint8:
            return "Uint8";
        case MonoTypes::Float:
            return "Float";
        case MonoTypes::Double:
            return "Double";
        case MonoTypes::String:
            return "String";
        case MonoTypes::Ptr:
            return "Ptr";
        case MonoTypes::ValueType:
            return "Value";
        case MonoTypes::Class:
            return "Class";
        case MonoTypes::Array:
            return "Array";
        case MonoTypes::Int:
            return "Int";
        case MonoTypes::Uint:
            return "Uint";
    }
    return "UNKNOWN";
}

MonoTypeMapper::MonoTypeMapper(MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr):
    _image(image),
    _memory(memory),
    _ptr(ptr) {

}

void MonoTypeMapper::loadInner() {
    _memory->readProcessMemory(&_data, _ptr + MONOTYPE_DATA_OFFSET);
    _memory->readProcessMemory(&_type, _ptr + MONOTYPE_TYPE_OFFSET);
    _memory->readProcessMemory(&_attrs, _ptr + MONOTYPE_ATTRS_OFFSET);

    // At this point we want to get some more information about the type.
    // Namely it's name which we can determine based off what's stored in _type.
    // Depending on its type: we might want to load additional data by parsing what's
    // stored in _data.
    _name = monoTypeToString(type());
    if (type() == MonoTypes::ValueType || type() == MonoTypes::Class) {
        _parsedData = _image->loadClassFromPtr(static_cast<uintptr_t>(_data));
    } else if (type() == MonoTypes::Ptr) {
        _parsedData = _image->loadTypeFromPtr(static_cast<uintptr_t>(_data));
    } else {
        _parsedData = nullptr;
    }
}

std::string MonoTypeMapper::name() const {
    std::ostringstream str;
    str << _name;

    if (!std::holds_alternative<nullptr_t>(_parsedData)) {
        str << "<";
        if (std::holds_alternative<const MonoTypeMapper*>(_parsedData)) {
            const auto* typ = std::get<const MonoTypeMapper*>(_parsedData);
            str << typ->name();
        } else {
            const auto* cls = std::get<const MonoClassMapper*>(_parsedData);
            str << cls->fullName();
        }
        str << ">";
    }
    return str.str();
}

std::ostream& operator<<(std::ostream& os, const MonoTypeMapper& map) {
    os << map.name();

    if (map.isPrivate()) {
        os << " | Private";
    }

    if (map.isPublic()) {
        os << " | Public";
    }

    if (map.isStatic()) {
        os << " | Static";
    }
    return os;
}

}