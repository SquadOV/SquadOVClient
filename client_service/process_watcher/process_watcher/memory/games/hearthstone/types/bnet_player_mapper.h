#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_account_id_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_account_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using BNetPlayerMapperSPtr = std::shared_ptr<class BNetPlayerMapper>;
class BNetPlayerMapper {
public:
    explicit BNetPlayerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    BNetAccountIdMapperSPtr accountId() const;
    BNetAccountMapperSPtr hsGameAccount() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

bool operator==(const BNetPlayerMapper& a, const BNetPlayerMapper& b);

}