#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using BNetAccountIdMapperSPtr = std::shared_ptr<class BNetAccountIdMapper>;
class BNetAccountIdMapper {
public:
    explicit BNetAccountIdMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    uint64_t hi() const;
    uint64_t lo() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

bool operator==(const BNetAccountIdMapper& a, const BNetAccountIdMapper& b);

}