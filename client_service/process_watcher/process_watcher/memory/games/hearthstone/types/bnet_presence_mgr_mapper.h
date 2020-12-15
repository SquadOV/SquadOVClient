#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_player_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using BNetPresenceMgrMapperSPtr = std::shared_ptr<class BNetPresenceMgrMapper>;
class BNetPresenceMgrMapper {
public:
    explicit BNetPresenceMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static BNetPresenceMgrMapperSPtr singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);

    BNetPlayerMapperSPtr myPlayer() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}