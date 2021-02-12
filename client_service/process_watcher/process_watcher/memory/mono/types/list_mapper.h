#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/mono_type_mapper.h"
#include "process_watcher/memory/mono/types/array_mapper.h"
#include <string>

namespace process_watcher::memory::mono::types {

template<typename T>
class ListMapper {
    inline static const std::string LIST_ITEMS_FIELD_NAME = "_items";
    inline static const std::string LIST_SIZE_FIELD_NAME = "_size";
public:
    ListMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
        _object(object)
    {}

    ArrayMapper<T> items() const {
        const auto value = _object->get(LIST_ITEMS_FIELD_NAME);
        return ArrayMapper<T>(
            value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>(),
            _object->fieldType(LIST_ITEMS_FIELD_NAME)
        );
    }

    int32_t size() const {
        const auto value = _object->get(LIST_SIZE_FIELD_NAME);
        return value.get<int32_t>();
    }

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};


}