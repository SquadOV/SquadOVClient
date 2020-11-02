#include "process_watcher/memory/games/hearthstone/types/game_mgr_mapper.h"

namespace mono = process_watcher::memory::mono;

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string GAMEMGR_FULL_NAME("GameMgr");
const std::string GAMEMGR_LASTDECKID_FIELD_NAME("m_lastDeckId");

}

const process_watcher::memory::mono::MonoClassMapper* GameMgrMapper::klass(const process_watcher::memory::mono::MonoImageMapper& image) {
    return image.loadClassFromFullName(GAMEMGR_FULL_NAME);
}

GameMgrMapper::GameMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

mono::types::NullableMapper<int64_t> GameMgrMapper::lastDeckId() const {
    const auto value = _object->get(GAMEMGR_LASTDECKID_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return mono::types::NullableMapper<int64_t>(value.get<mono::MonoObjectMapperSPtr>());
}

}