#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace process_watcher::memory::mono {

class MonoObjectMapper;
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
    std::vector<char>, // A generic byte array - we'll use this to store structs.
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
    const T& getDefault(const T& default) const {
        try {
            return std::get<T>(_internal);
        } catch(...) {
            return default;
        }
    }

    template<typename T>
    DynamicMonoType& operator=(const T& other) {
        _internal = other;
        return *this;
    }

private:
    RawDynamicMonoType _internal;
    uintptr_t _ptr;
};

}