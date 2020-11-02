#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include <string>

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string COLLECTION_DECK_NAME_FIELD_NAME("m_name");
const std::string COLLECTION_DECK_ID_FIELD_NAME("ID");
const std::string COLLECTION_DECK_HERO_CARD_FIELD_NAME("HeroCardID");
const std::string COLLECTION_DECK_HERO_PREMIUM_FIELD_NAME("HeroPremium");
const std::string COLLECTION_DECK_TYPE_FIELD_NAME("Type");
const std::string COLLECTION_DECK_CREATE_DATE_FIELD_NAME("CreateDate");
const std::string COLLECTION_DECK_IS_WILD_FIELD_NAME("m_isWild");
const std::string COLLECTION_DECK_SLOTS_FIELD_NAME("m_slots");

}

std::string deckTypeToString(DeckType typ) {
    switch (typ) {
        case DeckType::ClientOnly:
            return "Client Only";
        case DeckType::Normal:
            return "Normal";
        case DeckType::AI:
            return "AI";
        case DeckType::Draft:
            return "Draft";
        case DeckType::Precon:
            return "Preconstructed";
        case DeckType::TavernBrawl:
            return "Tavern Brawl";
        case DeckType::FsgBrawl:
            return "FsgBrawl";
        case DeckType::PvpDr:
            return "PvpDr";
        case DeckType::PvpDrDisplay:
            return "PvpDrDisplay";
        case DeckType::Hidden:
            return "Hidden";
        case DeckType::Unknown:
        default:
            return "Unknown";
    }
    return "";
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

CollectionDeckMapper::CollectionDeckMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

std::string CollectionDeckMapper::name() const {
    const auto value = _object->get(COLLECTION_DECK_NAME_FIELD_NAME);
    return value.get<std::string>();
}

int64_t CollectionDeckMapper::deckId() const {
    const auto value = _object->get(COLLECTION_DECK_ID_FIELD_NAME);
    return value.get<int64_t>();
}

std::string CollectionDeckMapper::heroCard() const {
    const auto value = _object->get(COLLECTION_DECK_HERO_CARD_FIELD_NAME);
    return value.get<std::string>();
}

TagPremium CollectionDeckMapper::heroPremium() const {
    const auto value = _object->get(COLLECTION_DECK_HERO_PREMIUM_FIELD_NAME);
    return static_cast<TagPremium>(value.get<int32_t>());
}

DeckType CollectionDeckMapper::deckType() const {
    const auto value = _object->get(COLLECTION_DECK_TYPE_FIELD_NAME);
    return static_cast<DeckType>(value.get<int32_t>());
}

uint64_t CollectionDeckMapper::createDate() const {
    const auto value = _object->get(COLLECTION_DECK_CREATE_DATE_FIELD_NAME);
    return value.get<uint64_t>();
}

bool CollectionDeckMapper::isWild() const {
    const auto value = _object->get(COLLECTION_DECK_IS_WILD_FIELD_NAME);
    return value.get<uint8_t>();
}

std::ostream& operator<<(std::ostream& os, const CollectionDeckMapper& map) {
    os << "Name: " << map.name()
       << std::endl << "DeckID: " << map.deckId()
       << std::endl << "Hero Card: " << map.heroCard() << " [" << tagPremiumToString(map.heroPremium()) << "]"
       << std::endl << "Type: " << deckTypeToString(map.deckType())
       << std::endl << "Create Date: " << map.createDate()
       << std::endl << "Is Wild: " << map.isWild();
       
    return os;
}

}