#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <memory>
#include <stdint.h>
#include <string>

namespace process_watcher::memory::games::hearthstone::types {

using UberTextSPtr = std::shared_ptr<class UberText>;
class UberText {
public:
    explicit UberText(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    
    std::string text() const;

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}