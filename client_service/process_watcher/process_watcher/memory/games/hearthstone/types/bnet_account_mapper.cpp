#include "process_watcher/memory/games/hearthstone/types/bnet_account_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string ID_FIELD_NAME("m_id");

}

BNetAccountMapper::BNetAccountMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

BNetAccountIdMapperSPtr BNetAccountMapper::id() const {
    const auto value = _object->get(ID_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<BNetAccountIdMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

bool operator==(const BNetAccountMapper& a, const BNetAccountMapper& b) {
    const auto aid = a.id();
    if (!aid) {
        return false;
    }
    const auto bid = b.id();
    if (!bid) {
        return false;
    }

    return *aid == *bid;
}

}