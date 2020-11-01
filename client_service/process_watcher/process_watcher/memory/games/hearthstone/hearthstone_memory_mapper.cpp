#include "process_watcher/memory/games/hearthstone/hearthstone_memory_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/deck_picker_tray_display_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_manager_mapper.h"

namespace process_watcher::memory::games::hearthstone {

HearthstoneMemoryMapper::HearthstoneMemoryMapper(process_watcher::memory::MonoMemoryMapperPtr&& mono):
    _mono(std::move(mono)) {

}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeck() const {
    auto deck = getCurrentDeckInUI();
    if (deck) {
        return deck;
    }
    return getCurrentDeckInMatch();
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeckInUI() const {
    auto deckPicker = types::DeckPickerTrayDisplayMapper::singleton(_mono->image(), _mono->domainId());
    if (!deckPicker) {
        return nullptr;
    }
    const auto deckVisual = deckPicker->getSelectedCustomDeckBox();
    return getDeckFromId(deckVisual->deckId());
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeckInMatch() const {
    return nullptr;
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getDeckFromId(int64_t deckId) const {
    auto mgr = types::CollectionManagerMapper::singleton(_mono->image(), _mono->domainId());
    if (!mgr) {
        return nullptr;
    }
    return mgr->getDeckFromId(deckId);
}

}