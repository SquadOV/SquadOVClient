#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_account_id_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_player_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using FriendlyChallengeDataMapperSPtr = std::shared_ptr<class FriendlyChallengeDataMapper>;
class FriendlyChallengeDataMapper {
public:
    explicit FriendlyChallengeDataMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    BNetAccountIdMapperSPtr challenger() const;
    int64_t challengerDeckId() const;

    BNetPlayerMapperSPtr challengee() const;
    int64_t challengeeDeckId() const;

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}