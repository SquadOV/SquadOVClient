#pragma once

#include "process_watcher/memory/mono_memory_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/player_mapper.h"
#include <memory>
#include <optional>

namespace process_watcher::memory::games::hearthstone {

// A typed wrapper around the MonoMemoryMapper that's specific to Hearthstone and
// exposes common operations to read data that we want to obtain in objects that
// are easily accessible. We expose functions that extract information that we want to
// external users of the HearthstoneMemoryMapper as opposed to exposing the mapped
// objects directly as I think that'll be easier to use.
class HearthstoneMemoryMapper {
public:
    explicit HearthstoneMemoryMapper(process_watcher::memory::MonoMemoryMapperPtr&& mono);

    // Gets the current deck. There's generally two ways to get the current deck -
    // either by what the user has selected in the UI on the deck selection screen
    // or what the user is actually playing in the match. This function will select
    // appropriately based on whatever state the user is in. Note that this function
    // *does not* keep track of state as it only reads in what's in memory at the
    // current moment.
    types::CollectionDeckMapperSPtr getCurrentDeck() const;
    types::CollectionDeckMapperSPtr getDeckFromId(int64_t deckId) const;
    types::CollectionDeckMapperSPtr getCurrentArenaDeck() const;
    types::CollectionDeckMapperSPtr getCurrentDuelsDeck() const;
    
    // Obtains information about the current players in the game.
    std::unordered_map<int, types::PlayerMapperSPtr> getCurrentPlayers() const;

    std::optional<int32_t> getBattlegroundsRating() const;
    std::optional<int32_t> getDuelsCasualRating() const;
    std::optional<int32_t> getDuelsHeroicRating() const;

private:
    types::CollectionDeckMapperSPtr getCurrentDeckInUI() const;
    // Note that this function will return the *last deck* used even if the game has ended.
    types::CollectionDeckMapperSPtr getCurrentDeckInMatch() const;

    process_watcher::memory::MonoMemoryMapperPtr _mono;
};
using HearthstoneMemoryMapperPtr = std::unique_ptr<HearthstoneMemoryMapper>;

}