#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/array_mapper.h"
#include "process_watcher/memory/mono/types/link_mapper.h"
#include <unordered_map>
#include <string>

namespace process_watcher::memory::mono::types {

template<typename T, typename V>
class MapMapper {
    inline static const std::string TOUCHED_SLOTS_FIELD_NAME = "touchedSlots";
    inline static const std::string LINK_SLOTS_FIELD_NAME = "linkSlots";
    inline static const std::string KEY_SLOTS_FIELD_NAME = "keySlots";
    inline static const std::string VALUE_SLOTS_FIELD_NAME = "valueSlots";

public:
    explicit MapMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
        _object(object)
    {
        // There's two options here, do an O(N) search here and use O(N) memory to store everything or
        // load stuff on demand but have to do an O(N) search every time we want to load something. Going
        // to load everything up front just for simplicity.
        //
        // The map is stored using 3 equally-sized arrays with 'touchedSlots' elements:
        // 1) linkSlots
        // 2) keySlots
        // 3) valueSlots
        // A "slot" has an element if it satisfies the following condition:
        //  ((uint)linkSlots[i].HashCode & 0x80000000u) != 0
        const auto touchedSlots = _object->get(TOUCHED_SLOTS_FIELD_NAME).get<int32_t>();

        const auto linkSlots = ArrayMapper<LinkMapper>(
            _object->get(LINK_SLOTS_FIELD_NAME).get<process_watcher::memory::mono::MonoObjectMapperSPtr>(),
            _object->fieldType(LINK_SLOTS_FIELD_NAME)
        );
        const auto keySlots = ArrayMapper<T>(
            _object->get(KEY_SLOTS_FIELD_NAME).get<process_watcher::memory::mono::MonoObjectMapperSPtr>(),
            _object->fieldType(KEY_SLOTS_FIELD_NAME)
        );
        const auto valueSlots = ArrayMapper<V>(
            _object->get(VALUE_SLOTS_FIELD_NAME).get<process_watcher::memory::mono::MonoObjectMapperSPtr>(),
            _object->fieldType(VALUE_SLOTS_FIELD_NAME)
        );

        for (int32_t i = 0; i < touchedSlots; ++i) {
            if (linkSlots[i].hashCode & 0x80000000u) {
                _values[keySlots[i]] = valueSlots[i];
            }
        }
    }

    bool exists(const T& key) const {
        return _values.find(key) != _values.end();
    }

    const V& get(const T& key) const {
        return _values.at(key);
    }

    const std::unordered_map<T,V>& values() const { return _values; }

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    std::unordered_map<T, V> _values;
};


}