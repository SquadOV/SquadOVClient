#include "process_watcher/memory/games/hearthstone/types/game_state_mapper.h"

namespace mono = process_watcher::memory::mono;
namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string GAME_STATE_CLASS_NAME = "GameState";
const std::string SINGLETON_FIELD_NAME = "s_instance";
const std::string PLAYER_MAP_FIELD_NAME = "m_playerMap";
const std::string GAMEOVER_FIELD_NAME = "m_gameOver";
}

GameStateMapperSPtr GameStateMapper::singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(GAME_STATE_CLASS_NAME);
    const auto& field = cls->field(SINGLETON_FIELD_NAME);
    const auto value = field.getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }

    const auto object = value.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<GameStateMapper>(object);
}

GameStateMapper::GameStateMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

typename GameStateMapper::PlayerMap GameStateMapper::playerMap() const {
    const auto object = _object->get(PLAYER_MAP_FIELD_NAME).get<mono::MonoObjectMapperSPtr>();
    return PlayerMap(object);
}

bool GameStateMapper::gameOver() const {
    return static_cast<bool>(_object->get(GAMEOVER_FIELD_NAME).get<uint8_t>());
}

}