#include "process_watcher/memory/games/hearthstone/types/game_session_info_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string HAS_CLIENT_DECK_ID_FIELD_NAME("HasClientDeckId");
const std::string CLIENT_DECK_ID_FIELD_NAME("_ClientDeckId");

}

GameSessionInfoMapper::GameSessionInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

bool GameSessionInfoMapper::hasClientDeckId() const {
    const auto value = _object->get(HAS_CLIENT_DECK_ID_FIELD_NAME);
    return static_cast<bool>(value.get<uint8_t>());
}

int64_t GameSessionInfoMapper::clientDeckId() const {
    const auto value = _object->get(CLIENT_DECK_ID_FIELD_NAME);
    return value.get<int64_t>();
}

}