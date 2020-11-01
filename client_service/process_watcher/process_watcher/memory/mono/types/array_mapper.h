#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_class_mapper.h"
#include "process_watcher/memory/mono/mono_type_mapper.h"
#include "process_watcher/memory/mono/types/struct.h"
#include <unordered_map>
#include <string>
#include <type_traits>

namespace process_watcher::memory::mono::types {

template<typename T>
class ArrayMapper;

template<typename T, typename D = void>
struct ArrayEleMapperHelper {
    ArrayEleMapperHelper(const ArrayMapper<T>& arr):
        _arr(arr)
    {}

    T extract(uintptr_t ptr) const;

private:
    const ArrayMapper<T>& _arr;
};


// This class maps the _MonoArray struct.
// struct _MonoArray {
// 	(+0) MonoObject obj;
// 	/* bounds is NULL for szarrays */
// 	(+8) MonoArrayBounds *bounds;
// 	/* total number of elements of the array */
// 	(+12) mono_array_size_t max_length; 
// 	/* we use double to ensure proper alignment on platforms that need it */
// 	(+16) double vector [MONO_ZERO_LEN_ARRAY];
// };
template<typename T>
class ArrayMapper {
    inline static const uint32_t ARRAY_LENGTH_OFFSET = 12;
    inline static const uint32_t ARRAY_DATA_OFFSET = 16;
public:
    ArrayMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object, const process_watcher::memory::mono::MonoTypeMapper* typ):
        _object(object),
        _type(typ)
    {
        _length = _object->memory()->readProcessMemory<uint32_t>(static_cast<uintptr_t>(_object->ptr() + ARRAY_LENGTH_OFFSET));
        _eleSize = _object->klass()->sizes();
        _eklass = _type->inner<const process_watcher::memory::mono::MonoClassMapper*>();
    }

    template<typename TIndex>
    T operator[](TIndex i) const {
        if (i < 0 || static_cast<uint32_t>(i) >= _length) {
            THROW_ERROR("Index out of bounds: " << i << std::endl);
        }

        const uintptr_t ptr = _object->ptr() + ARRAY_DATA_OFFSET + i * _eleSize;
        ArrayEleMapperHelper<T> helper(*this);
        return helper.extract(ptr);
    }

    friend struct ArrayEleMapperHelper<T>;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    const process_watcher::memory::mono::MonoTypeMapper* _type;
    const process_watcher::memory::mono::MonoClassMapper* _eklass;

    uint32_t _length;
    int32_t _eleSize;
};

template<typename T, typename D>
T ArrayEleMapperHelper<T, D>::extract(uintptr_t ptr) const {
    if (_arr._eklass->isValueType()) {
        return T(std::make_shared<process_watcher::memory::mono::MonoObjectMapper>(
            _arr._object->image(),
            _arr._object->memory(),
            ptr,
            _arr._object->domainId()
        ));
    } else {
        const auto newPtr = _arr._object->memory()->readProcessMemory<uint32_t>(ptr);
        return T(std::make_shared<process_watcher::memory::mono::MonoObjectMapper>(
            _arr._object->image(),
            _arr._object->memory(),
            newPtr,
            _arr._object->domainId()
        ));
    }
    return T();
}

template<typename T>
struct ArrayEleMapperHelper<T,
    typename std::enable_if_t<
        std::is_arithmetic_v<T> ||
        std::is_base_of_v<types::RawStruct, T>
    >
> {
    ArrayEleMapperHelper(const ArrayMapper<T>& arr):
        _arr(arr)
    {}

    T extract(uintptr_t ptr) const {
        assert(sizeof(T) == _arr._eleSize);
        assert(_arr._eklass->isValueType());
        return _arr._object->memory()->readProcessMemory<T>(ptr);
    }

private:
    const ArrayMapper<T>& _arr;
};

}