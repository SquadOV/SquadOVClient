#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <string>

namespace process_watcher::memory::mono::types {

class StringMapper {
public:
    explicit StringMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    const std::string& value() const { return _value; }
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    std::string _value;
};


}