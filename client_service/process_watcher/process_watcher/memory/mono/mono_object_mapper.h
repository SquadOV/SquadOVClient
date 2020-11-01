#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include <memory>
#include <string>
#include <variant>

namespace process_watcher::memory::mono {

using MonoObjectMapperSPtr = std::shared_ptr<class MonoObjectMapper>;

using RawDynamicMonoType = std::variant<
    MonoObjectMapperSPtr,
    std::string, // Conversion to UTF-8 for ourselves
    char16_t,    // Actual representation of the UTF-16 string by C#
    int8_t,
    uint8_t,
    int16_t,
    uint16_t,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float,
    double,
    nullptr_t
>;

class DynamicMonoType {
public:
    DynamicMonoType():
        _internal(nullptr),
        _ptr(0)
    {}

    explicit DynamicMonoType(nullptr_t):
        _internal(nullptr),
        _ptr(0)
    {}

    explicit DynamicMonoType(uintptr_t ptr):
        _internal(nullptr),
        _ptr(ptr)
    {}

    uintptr_t ptr() const { return _ptr; }
    bool isNull() const { return std::holds_alternative<nullptr_t>(_internal); }

    template<typename T>
    const T& get() const { return std::get<T>(_internal); }

    template<typename T>
    DynamicMonoType& operator=(const T& other) {
        _internal = other;
        return *this;
    }

private:
    RawDynamicMonoType _internal;
    uintptr_t _ptr;
};

class MonoObjectMapper {
public:
    MonoObjectMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr, int32_t domainId);

    uintptr_t ptr() const { return _ptr; }
    const process_watcher::memory::ModuleMemoryMapperSPtr& memory() const { return _memory; }
    MonoImageMapper* image() const { return _image; }
    const class MonoClassMapper* klass() const { return _klass; }
    int32_t domainId() const { return _domainId; }

    // Convenience functions to obtain field values for this specific object.
    DynamicMonoType get(const std::string& fieldName) const;
    const class MonoTypeMapper* fieldType(const std::string& fieldName) const;
private:
    class MonoImageMapper* _image;
    const class MonoVTableMapper* _vtable;
    const class MonoClassMapper* _klass;
    int32_t _domainId;

    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr;
};


}