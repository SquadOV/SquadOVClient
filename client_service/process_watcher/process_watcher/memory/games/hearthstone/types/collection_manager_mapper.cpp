#include "process_watcher/memory/games/hearthstone/types/collection_manager_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"
#include <string>

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string COLLECTION_MANAGER_FULLNAME("CollectionManager");
const std::string COLLECTION_MANAGER_SINGLETON_FIELD("s_instance");
const std::string COLLECTION_MANAGER_DECKS_FIELD("m_decks");
const std::string COLLECTION_MANAGER_BASE_DECKS_FIELD("m_baseDecks");
const std::string COLLECTION_MANAGER_CURRENT_PVPDR_DECK_ID_FIELD("m_currentPVPDRDeckId");

}
CollectionManagerMapperSPtr CollectionManagerMapper::singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(COLLECTION_MANAGER_FULLNAME);
    const auto* field = cls->field(COLLECTION_MANAGER_SINGLETON_FIELD);
    if (!field) {
        return nullptr;
    }
    const auto value = field->getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }

    const auto object = value.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<CollectionManagerMapper>(object);
}

CollectionManagerMapper::CollectionManagerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

CollectionDeckMapperSPtr CollectionManagerMapper::getDeckFromId(int64_t deckId) const {
    auto deck = getDeckFromIdFromDeckMap(COLLECTION_MANAGER_DECKS_FIELD, deckId);
    if (deck) {
        return deck;
    }

    return getDeckFromIdFromDeckMap(COLLECTION_MANAGER_BASE_DECKS_FIELD, deckId);
}

CollectionDeckMapperSPtr CollectionManagerMapper::getCurrentDuelsDeck() const {
    const auto deckId = _object->get(COLLECTION_MANAGER_CURRENT_PVPDR_DECK_ID_FIELD).get<int64_t>();
    return getDeckFromId(deckId);
}

CollectionDeckMapperSPtr CollectionManagerMapper::getDeckFromIdFromDeckMap(const std::string& fieldName, int64_t deckId) const {
    auto deckMap = getDeckMap(fieldName);
    if (!deckMap.exists(deckId)) {
        return nullptr;
    }
    return std::make_shared<CollectionDeckMapper>(deckMap.get(deckId));
}

CollectionManagerMapper::DeckMap CollectionManagerMapper::getDeckMap(const std::string& fieldName) const {
    const auto selectedCustomDeckBox = _object->get(fieldName);
    const auto object = selectedCustomDeckBox.get<mono::MonoObjectMapperSPtr>();
    return DeckMap(object);
}

}