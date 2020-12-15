#include "process_watcher/memory/games/hearthstone/types/friendly_challenge_data_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string CHALLENGER_FIELD_NAME("m_challengerId");
const std::string CHALLENGER_DECK_ID_FIELD_NAME("m_challengerDeckId");

const std::string CHALLENGEE_FIELD_NAME("m_challengee");
const std::string CHALLENGEE_DECK_ID_FIELD_NAME("m_challengeeDeckId");
}

FriendlyChallengeDataMapper::FriendlyChallengeDataMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

BNetAccountIdMapperSPtr FriendlyChallengeDataMapper::challenger() const {
    const auto value = _object->get(CHALLENGER_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<BNetAccountIdMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

int64_t FriendlyChallengeDataMapper::challengerDeckId() const {
    const auto value = _object->get(CHALLENGER_DECK_ID_FIELD_NAME);
    return value.get<int64_t>();
}

BNetPlayerMapperSPtr FriendlyChallengeDataMapper::challengee() const {
    const auto value = _object->get(CHALLENGEE_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<BNetPlayerMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

int64_t FriendlyChallengeDataMapper::challengeeDeckId() const {
    const auto value = _object->get(CHALLENGEE_DECK_ID_FIELD_NAME);
    return value.get<int64_t>();
}

}