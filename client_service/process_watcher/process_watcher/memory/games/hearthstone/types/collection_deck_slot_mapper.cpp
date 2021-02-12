#include "process_watcher/memory/games/hearthstone/types/collection_deck_slot_mapper.h"

namespace mono = process_watcher::memory::mono;
namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string INDEX_FIELD_NAME = "m_index";
const std::string COUNT_FIELD_NAME = "m_count";
const std::string CARD_ID_FIELD_NAME = "m_cardId";
const std::string OWNED_FIELD_NAME = "m_owned";

}

std::string tagPremiumToString(TagPremium typ) {
    switch (typ) {
        case TagPremium::Normal:
            return "Normal";
        case TagPremium::Golden:
            return "Golden";
    }
    return "";
}

CollectionDeckSlotMapper::CollectionDeckSlotMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
}

int32_t CollectionDeckSlotMapper::index() const {
    const auto value = _object->get(INDEX_FIELD_NAME);
    return value.get<int32_t>();
}

std::string CollectionDeckSlotMapper::cardId() const {
    const auto value = _object->get(CARD_ID_FIELD_NAME);
    return value.get<std::string>();
}

bool CollectionDeckSlotMapper::owned() const {
    const auto value = _object->get(OWNED_FIELD_NAME);
    return value.get<uint8_t>();
}

int32_t CollectionDeckSlotMapper::count() const {
    const auto cnts = allCounts().items();
    return cnts[0] + cnts[1];
}

int32_t CollectionDeckSlotMapper::count(TagPremium premium) const {
    const auto cnts = allCounts().items();
    return cnts[static_cast<int>(premium)];
}

mono::types::ListMapper<int32_t> CollectionDeckSlotMapper::allCounts() const {
    const auto value = _object->get(COUNT_FIELD_NAME);
    return mono::types::ListMapper<int32_t>(value.get<mono::MonoObjectMapperSPtr>());
}

nlohmann::json CollectionDeckSlotMapper::toJson() const {
    return {
        { "index", index() },
        { "cardId", cardId() },
        { "owned", owned() },
        { "count", {
            { "normal", count(TagPremium::Normal) },
            { "golden", count(TagPremium::Golden) },
        }}
    };
}

std::ostream& operator<<(std::ostream& os, const CollectionDeckSlotMapper& map) {
    os << "Card: " << map.cardId()
       << std::endl << "Index: " << map.index()
       << std::endl << "Owned: " << map.owned()
       << std::endl << "Count : " << map.count()
       << std::endl << "\tNormal: " << map.count(TagPremium::Normal)
       << std::endl << "\tGolden: " << map.count(TagPremium::Golden);
    return os;
}

}