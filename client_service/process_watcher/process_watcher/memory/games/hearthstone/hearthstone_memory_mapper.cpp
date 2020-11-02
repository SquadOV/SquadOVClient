#include "process_watcher/memory/games/hearthstone/hearthstone_memory_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/deck_picker_tray_display_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_manager_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/hearthstone_services_mapper.h"

namespace process_watcher::memory::games::hearthstone {

HearthstoneMemoryMapper::HearthstoneMemoryMapper(process_watcher::memory::MonoMemoryMapperPtr&& mono):
    _mono(std::move(mono)) {

}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeck() const {
    const auto deck = getCurrentDeckInUI();
    if (deck) {
        return deck;
    }
    return getCurrentDeckInMatch();
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeckInUI() const {
    const auto deckPicker = types::DeckPickerTrayDisplayMapper::singleton(_mono->image(), _mono->domainId());
    if (!deckPicker) {
        return nullptr;
    }
    const auto deckVisual = deckPicker->getSelectedCustomDeckBox();
    return getDeckFromId(deckVisual->deckId());
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeckInMatch() const {
    const auto gameMgr = types::HearthstoneServicesMapper::getGameMgr(_mono->image(), _mono->domainId());
    if (!gameMgr) {
        return nullptr;
    }

    const auto deckId = gameMgr->lastDeckId();
    if (!deckId) {
        return nullptr;
    }

    return getDeckFromId(deckId.value());
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getDeckFromId(int64_t deckId) const {
    const auto mgr = types::CollectionManagerMapper::singleton(_mono->image(), _mono->domainId());
    if (!mgr) {
        return nullptr;
    }
    return mgr->getDeckFromId(deckId);
}

}