#include "process_watcher/memory/games/hearthstone/types/bnet_player_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string ACCOUNT_ID_FIELD_NAME("m_accountId");
const std::string HS_GAME_ACCOUNT_FIELD_NAME("m_hsGameAccount");

}

BNetPlayerMapper::BNetPlayerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

BNetAccountIdMapperSPtr BNetPlayerMapper::accountId() const {
    const auto value = _object->get(ACCOUNT_ID_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<BNetAccountIdMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

BNetAccountMapperSPtr BNetPlayerMapper::hsGameAccount() const {
    const auto value = _object->get(HS_GAME_ACCOUNT_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<BNetAccountMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

bool operator==(const BNetPlayerMapper& a, const BNetPlayerMapper& b) {
    const auto aId = a.accountId();
    if (!aId) {
        return false;
    }
    const auto bId = b.accountId();
    if (!bId) {
        return false;
    }
    return *aId == *bId;
}

}