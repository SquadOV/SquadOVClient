#include "process_watcher/memory/games/hearthstone/types/find_game_result_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string SESSION_FIELD_NAME("_GameSessionInfo");

}

FindGameResultMapper::FindGameResultMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

GameSessionInfoMapperSPtr FindGameResultMapper::getSessionInfo() const { 
    const auto value = _object->get(SESSION_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<GameSessionInfoMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

}