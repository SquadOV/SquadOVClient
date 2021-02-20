#pragma once

#include "process_watcher/memory/mono/mono_forward.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/map_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/player_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using GameStateMapperSPtr = std::shared_ptr<class GameStateMapper>;
class GameStateMapper {
public:
    static GameStateMapperSPtr singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);

    explicit GameStateMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    using PlayerMap = process_watcher::memory::mono::types::MapMapper<int, PlayerMapperSPtr>;
    PlayerMap playerMap() const;
    bool gameOver() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}