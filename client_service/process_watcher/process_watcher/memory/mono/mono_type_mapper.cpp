#include "process_watcher/memory/mono/mono_type_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

#include <sstream>
#include <chrono>
#include <thread>

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONOTYPE_DATA_OFFSET = 0;
constexpr uint32_t MONOTYPE_ATTRS_OFFSET = 4;
constexpr uint32_t MONOTYPE_TYPE_OFFSET = 6;
constexpr uint32_t MONOTYPE_OTHER_OFFSET = 7;
constexpr uint8_t MONOTYPE_BYREF_BITFLAG = 0b01000000;

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
        case MonoTypes::Var:
            return "Var";
        case MonoTypes::Array:
            return "Array";
        case MonoTypes::GenericInst:
            return "GenericInst";
        case MonoTypes::Int:
            return "Int";
        case MonoTypes::Uint:
            return "Uint";
        case MonoTypes::SzArray:
            return "Array";
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

    // This covers num_mods, byref, and pinned. Note that these bitflags
    // are stored in the *opposite* order of how they appear in the C struct.
    uint8_t otherFlags = 0;
    _memory->readProcessMemory(&otherFlags, _ptr + MONOTYPE_OTHER_OFFSET);
    _byRef = otherFlags & MONOTYPE_BYREF_BITFLAG;

    // At this point we want to get some more information about the type.
    // Namely it's name which we can determine based off what's stored in _type.
    // Depending on its type: we might want to load additional data by parsing what's
    // stored in _data.
    _name = monoTypeToString(type());
    if (type() == MonoTypes::ValueType || type() == MonoTypes::Class) {
        _parsedData = _image->loadClassFromPtr(static_cast<uintptr_t>(_data));
    } else if (type() == MonoTypes::Ptr) {
        _parsedData = _image->loadTypeFromPtr(static_cast<uintptr_t>(_data));
    } else if (type() == MonoTypes::GenericInst) {
        // 'data' is a pointer to a GenericClass whose first element is the pointer to the
        // containing class.
        const auto genericClassPtr = _memory->readProcessMemory<uint32_t>(static_cast<uintptr_t>(_data));
        _parsedData = _image->loadClassFromPtr(static_cast<uintptr_t>(genericClassPtr));

        // After the pointer to the container class comes the MonoGenericContext which is just two pointers,
        // one to a MonoGenericInst for class generic parameters and another for method generic parameters.
        // We're going to go ahead and just assume this is going to be a class generic since we're not loading
        // methods at this point.
        const auto genericInstPtr = _memory->readProcessMemory<uint32_t>(static_cast<uintptr_t>(_data + 4));

        // A MonoGenericInst looks as follows:
        // struct _MonoGenericInst {
        //    #ifndef MONO_SMALL_CONFIG
        //        gint32 id;			/* unique ID for debugging */
        //    #endif
        //        guint type_argc    : 22;	/* number of type arguments */
        //        guint is_open      :  1;	/* if this is an open type */
        //        MonoType *type_argv [MONO_ZERO_LEN_ARRAY];
        //  };
        // So we just need to load the MonoTypes in the type_argv array.
        const auto argc = _memory->readProcessMemory<uint32_t>(static_cast<uintptr_t>(genericInstPtr + 4));
        const uint32_t numTypes = argc & 0x3FFFFF; // Only look at the most significant 22 bits
        for (uint32_t i = 0; i < numTypes; ++i) {
            const auto typePtr = _memory->readProcessMemory<uint32_t>(static_cast<uintptr_t>(genericInstPtr + 8 + i * 4));
            _genericTypes.push_back(_image->loadTypeFromPtr(static_cast<uintptr_t>(typePtr)));
        }
    } else if (type() == MonoTypes::SzArray) {
        // data points to a MonoArrayType whose first element is a pointer to the MonoClass that
        // makes up the array.
        const auto klassPtr = _memory->readProcessMemory<uint32_t>(static_cast<uintptr_t>(_data));
        _parsedData = _image->loadClassFromPtr(static_cast<uintptr_t>(klassPtr));
    } else {
        _parsedData = nullptr;
    }
}

std::string MonoTypeMapper::name() const {
    std::ostringstream str;

    if (byRef()) {
        str << "&";
    }

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

        if (!_genericTypes.empty()) {
            str << "<";
            for (const auto& typ : _genericTypes) {
                str << typ->name() << ", ";
            }
            str << ">";
        }

        str << ">";
    }
    return str.str();
}

bool operator==(const MonoTypeMapper& a, const MonoTypeMapper& b) {
    // I'm not sure if it's valid to just compare the "data" but yolooooo.
    return a.rawData() == b.rawData();
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

    if (map.isLiteral()) {
        os << " | Literal";
    }
    return os;
}

}