#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/friendly_challenge_data_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using FriendlyChallengeMgrMapperSPtr = std::shared_ptr<class FriendlyChallengeMgrMapper>;
class FriendlyChallengeMgrMapper {
public:
    explicit FriendlyChallengeMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static FriendlyChallengeMgrMapperSPtr singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);

    FriendlyChallengeDataMapperSPtr data() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}