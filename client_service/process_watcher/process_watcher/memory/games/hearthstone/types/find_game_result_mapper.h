#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/game_session_info_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

class FindGameResultMapper {
public:
    explicit FindGameResultMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    GameSessionInfoMapperSPtr getSessionInfo() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};
using FindGameResultMapperSPtr = std::shared_ptr<FindGameResultMapper>;

}