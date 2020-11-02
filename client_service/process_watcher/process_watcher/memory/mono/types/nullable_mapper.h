#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/struct.h"
#include <string>
#include <type_traits>

namespace process_watcher::memory::mono::types {

template<typename T, std::enable_if_t<
    std::is_arithmetic_v<T> ||
    std::is_base_of_v<types::RawStruct, T>,
    int
> = 0>
class NullableMapper {
public:
    NullableMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
        _object(object)
    {
        // Don't go through the usual route of grabbing the value and instead
        // choose to read directly from memory. Why do this? Because I honestly
        // am not sure how to handle the generic ness of the MONO_TYPE_VAR in there AND
        // the Nullable type is STRAAAAAAAAAAAAAAANGE. I *do not* know why the field
        // offsets can't be used here. For example, for a Nullable<long>, the value
        // offset is 8 and the has_value offset is 12. I DUNNO. But from looking at the function
        // "mono_unity_object_unbox_nullable", this is probalby actually the right way
        // to read things instead of going through the fields anyway.
        _hasValue = _object->memory()->readProcessMemory<uint8_t>(_object->ptr() + sizeof(T));
        _value = _object->memory()->readProcessMemory<T>(_object->ptr());
    }
    
    NullableMapper(nullptr_t) {}

    operator bool() const { return _hasValue; }
    const T& value() const { return _value; }

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    bool _hasValue = false;
    T _value;
};

}