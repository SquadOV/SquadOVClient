#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_account_id_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using BNetAccountMapperSPtr = std::shared_ptr<class BNetAccountMapper>;
class BNetAccountMapper {
public:
    explicit BNetAccountMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    BNetAccountIdMapperSPtr id() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

bool operator==(const BNetAccountMapper& a, const BNetAccountMapper& b);

}