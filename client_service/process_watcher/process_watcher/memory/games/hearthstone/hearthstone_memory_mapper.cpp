#include "process_watcher/memory/games/hearthstone/hearthstone_memory_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/deck_picker_tray_display_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_manager_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/hearthstone_services_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/game_state_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/friendly_challenge_mgr_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_presence_mgr_mapper.h"

namespace process_watcher::memory::games::hearthstone {

HearthstoneMemoryMapper::HearthstoneMemoryMapper(process_watcher::memory::MonoMemoryMapperPtr&& mono):
    _mono(std::move(mono)) {

}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeck() const {
    //const auto deck = getCurrentDeckInUI();
    //if (deck) {
    //    return deck;
    //}
    return getCurrentDeckInMatch();
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeckInUI() const {
    const auto deckPicker = types::DeckPickerTrayDisplayMapper::singleton(_mono->image(), _mono->domainId());
    if (!deckPicker) {
        return nullptr;
    }
    const auto deckVisual = deckPicker->getSelectedCustomDeckBox();
    if (!deckVisual) {
        return nullptr;
    }
    return getDeckFromId(deckVisual->deckId());
}

types::BNetAccountIdMapperSPtr HearthstoneMemoryMapper::getLocalBNetPlayer() const {
    // Ideally we'd use BnetPresenceMgr but I can't for the life of me figure out why it's not showing up as
    // a class when I can find it in ILSpy.
    for (const auto& kvp : getCurrentPlayers()) {
        if (kvp.second->local()) {
            return kvp.second->gameAccountId();
        }
    }
    return nullptr;
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDeckInMatch() const {
    const auto gameMgr = types::HearthstoneServicesMapper::getGameMgr(_mono->image(), _mono->domainId());
    if (!gameMgr) {
        return nullptr;
    }

    // If the current game is constructed then we return the match made last deck ID.
    // If the current game is arena, then return the arena deck.
    if (gameMgr->gameType() == types::GameType::GT_ARENA) {
        return getCurrentArenaDeck();
    } else if (gameMgr->gameType() == types::GameType::GT_PVPDR || gameMgr->gameType() == types::GameType::GT_PVPDR_PAID) {
        return getCurrentDuelsDeck();
    } else {
        const auto deckId = gameMgr->lastDeckId();
        if (deckId) {
            return getDeckFromId(deckId.value());
        }

        // At this point we are probably in a friendly match
        // which is handled separately from the regular GameMgr.
        const auto friendly = types::FriendlyChallengeMgrMapper::singleton(_mono->image(), _mono->domainId());
        if (!friendly) {
            return nullptr;
        }

        const auto friendlyData = friendly->data();
        if (!friendlyData) {
            return nullptr;
        }

        const auto myPlayer = getLocalBNetPlayer();
        if (!myPlayer) {
            return nullptr;
        }

        const auto challenger = friendlyData->challenger();
        if (challenger && *challenger == *myPlayer) {
            return getDeckFromId(friendlyData->challengerDeckId());
        }

        const auto challengee = friendlyData->challengee();
        if (challengee && *challengee->hsGameAccount()->id() == *myPlayer) {
            return getDeckFromId(friendlyData->challengeeDeckId());
        }

        return nullptr;
    }
}

bool HearthstoneMemoryMapper::isSpectator() const {
    const auto gameMgr = types::HearthstoneServicesMapper::getGameMgr(_mono->image(), _mono->domainId());
    if (!gameMgr) {
        return false;
    }
    return gameMgr->isSpectator();
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentArenaDeck() const {
    const auto draftManager = types::HearthstoneServicesMapper::getDraftManager(_mono->image(), _mono->domainId());
    if (!draftManager) {
        return nullptr;
    }
    return draftManager->getDraftDeck();
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getCurrentDuelsDeck() const {
    const auto mgr = types::CollectionManagerMapper::singleton(_mono->image(), _mono->domainId());
    if (!mgr) {
        return nullptr;
    }
    return mgr->getCurrentDuelsDeck();
}

types::CollectionDeckMapperSPtr HearthstoneMemoryMapper::getDeckFromId(int64_t deckId) const {
    const auto mgr = types::CollectionManagerMapper::singleton(_mono->image(), _mono->domainId());
    if (!mgr) {
        return nullptr;
    }
    return mgr->getDeckFromId(deckId);
}

std::unordered_map<int, types::PlayerMapperSPtr> HearthstoneMemoryMapper::getCurrentPlayers() const {
    const auto state = types::GameStateMapper::singleton(_mono->image(), _mono->domainId());
    if (!state) {
        return {};
    }

    const auto playerMap = state->playerMap();
    return playerMap.values();
}

std::optional<int32_t> HearthstoneMemoryMapper::getBattlegroundsRating() const {
    const auto cache = types::HearthstoneServicesMapper::getNetCache(_mono->image(), _mono->domainId());
    if (!cache) {
        return std::nullopt;
    }

    const auto rating = cache->getBaconRatingInfo();
    if (!rating) {
        return std::nullopt;
    }
    return rating->rating();
}

std::optional<int32_t> HearthstoneMemoryMapper::getDuelsCasualRating() const {
    const auto cache = types::HearthstoneServicesMapper::getNetCache(_mono->image(), _mono->domainId());
    if (!cache) {
        return std::nullopt;
    }

    const auto rating = cache->getPvpDrStatsInfo();
    if (!rating) {
        return std::nullopt;
    }
    return rating->rating();
}

std::optional<int32_t> HearthstoneMemoryMapper::getDuelsHeroicRating() const {
    const auto cache = types::HearthstoneServicesMapper::getNetCache(_mono->image(), _mono->domainId());
    if (!cache) {
        return std::nullopt;
    }

    const auto rating = cache->getPvpDrStatsInfo();
    if (!rating) {
        return std::nullopt;
    }
    return rating->paidRating();
}

}