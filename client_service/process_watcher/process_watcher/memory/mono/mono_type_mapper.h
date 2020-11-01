#pragma once

#include "process_watcher/memory/module_memory_mapper.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace process_watcher::memory::mono {

enum class MonoTypeAttributes {
    Private = 0x0001,
    Public = 0x0006,
    Static = 0x0010,
    Literal = 0x0040
};

enum class MonoTypes {
    Void = 0x01,
    Bool = 0x02,
    Char = 0x03,
    Int1 = 0x04,
    Uint1 = 0x05,
    Int2 = 0x06,
    Uint2 = 0x07,
    Int4 = 0x08,
    Uint4 = 0x09,
    Int8 = 0x0a,
    Uint8 = 0x0b,
    Float = 0x0c,
    Double = 0x0d,
    String = 0x0e,
    Ptr = 0x0f,
    ValueType = 0x11,
    Class = 0x12,
    Array = 0x14,
    GenericInst = 0x15,
    Int = 0x18,
    Uint = 0x19,
    SzArray = 0x1d,
};

std::string monoTypeToString(MonoTypes typ);

// Pulls data out of memory that represents a MonoType.
class MonoTypeMapper {
public:
    MonoTypeMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr);
    void loadInner();

    bool isPrivate() const { return _attrs & static_cast<uint16_t>(MonoTypeAttributes::Private); }
    bool isPublic() const { return _attrs & static_cast<uint16_t>(MonoTypeAttributes::Public); }
    bool isStatic() const { return _attrs & static_cast<uint16_t>(MonoTypeAttributes::Static); }
    bool isLiteral() const { return _attrs & static_cast<uint16_t>(MonoTypeAttributes::Literal); }

    MonoTypes type() const { return static_cast<MonoTypes>(_type); }
    bool byRef() const { return _byRef; }

    std::string name() const;
    friend std::ostream& operator<<(std::ostream& os, const MonoTypeMapper& map);

    template<typename T>
    T inner() const {
        return std::get<T>(_parsedData);
    }
    
private:
    class MonoImageMapper* _image;
    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr = 0;

    uint32_t _data;
    uint16_t _attrs;
    uint8_t _type;
    // Whether or not this type should be accessed by ref (aka what's stored in memory is a pointer, effectively).
    bool _byRef;

    // We'll be loading some sort of pointer into _data. If that pointers to
    // something that we have mapped (a pointer to a type for example) then
    // store a reference to that here.
    std::variant<const MonoTypeMapper*, const class MonoClassMapper*, std::nullptr_t> _parsedData;

    // If this is a generic, this generic types array will be filled with the generic type params.
    std::vector<const MonoTypeMapper*> _genericTypes;

    // A human readable name for the type that we generate.
    std::string _name;
};

std::ostream& operator<<(std::ostream& os, const MonoTypeMapper& map);
using MonoTypeMapperPtr = std::unique_ptr<MonoTypeMapper>;

}