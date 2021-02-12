#include "process_watcher/memory/games/hearthstone/types/bnet_account_id_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string PARENT_CLASS_NAME("BnetEntityId");
const std::string HI_FIELD_NAME("m_hi");
const std::string LO_FIELD_NAME("m_lo");

}

BNetAccountIdMapper::BNetAccountIdMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

uint64_t BNetAccountIdMapper::hi() const {
    const auto value = _object->superGet(PARENT_CLASS_NAME, HI_FIELD_NAME);
    return value.get<uint64_t>();
}

uint64_t BNetAccountIdMapper::lo() const {
    const auto value = _object->superGet(PARENT_CLASS_NAME, LO_FIELD_NAME);
    return value.get<uint64_t>();
}

bool operator==(const BNetAccountIdMapper& a, const BNetAccountIdMapper& b) {
    return a.hi() == b.hi() && a.lo() == b.lo();
}

}