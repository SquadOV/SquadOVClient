#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include <string>

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string COLLECTION_DECK_NAME_FIELD_NAME("m_name");
const std::string COLLECTION_DECK_ID_FIELD_NAME("ID");

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

}