#include "process_watcher/memory/games/hearthstone/types/network_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {
const std::string NETWORKMAPPER_FULL_NAME("Network");
const std::string STATE_FIELD("m_state");

const std::string LAST_FIND_GAME_PARAMETERS_FIELD("<LastFindGameParameters>k__BackingField");
}

NetworkMapper::NetworkMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

const process_watcher::memory::mono::MonoClassMapper* NetworkMapper::klass(const process_watcher::memory::mono::MonoImageMapper& image) {
    return image.loadClassFromFullName(NETWORKMAPPER_FULL_NAME);
}

NetworkMapper::NetworkStateMapperSPtr NetworkMapper::getState() const {
    const auto value = _object->get(STATE_FIELD);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<NetworkStateMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

NetworkMapper::NetworkStateMapper::NetworkStateMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

FindGameResultMapperSPtr NetworkMapper::NetworkStateMapper::getLastFindGameParameters() const {
    const auto value = _object->get(LAST_FIND_GAME_PARAMETERS_FIELD);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<FindGameResultMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

}