#include "process_watcher/memory/games/hearthstone/types/collection_deck_box_visual_mapper.h"
#include <string>

using namespace process_watcher::memory;

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string DECKBOX_DECK_NAME_FIELD_NAME("m_deckName");
const std::string DECKBOX_DECK_ID_FIELD_NAME("m_deckID");

}

CollectionDeckBoxVisualMapper::CollectionDeckBoxVisualMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
}

UberTextSPtr CollectionDeckBoxVisualMapper::deckName() const {
    const auto value = _object->get(DECKBOX_DECK_NAME_FIELD_NAME);
    return std::make_shared<UberText>(value.get<mono::MonoObjectMapperSPtr>());
}

int64_t CollectionDeckBoxVisualMapper::deckId() const {
    const auto value = _object->get(DECKBOX_DECK_ID_FIELD_NAME);
    return value.get<int64_t>();
}

}