#include "game_event_watcher/csgo/csgo_gsi_state_manager.h"
#include "game_event_watcher/csgo/csgo_gsi_listener.h"

namespace game_event_watcher {

bool CsgoMatchState::isSupported() const {
    return (mode == "casual" || mode == "competitive" || "scrimcomp2v2");
}

CsgoWeaponState CsgoWeaponState::from(const CsgoGsiWeaponPacket& packet) {
    CsgoWeaponState state;
    state.name = packet.name.value_or("");
    state.paintkit = packet.paintkit.value_or("");
    state.type = packet.type.value_or("");
    return state;
}

CsgoPlayerRoundState CsgoPlayerRoundState::from(const CsgoGsiPlayerPacket& packet) {
    CsgoPlayerRoundState state;
    state.steamId = packet.steamId.value_or("");
    state.team = packet.team.value_or("");

    if (packet.state) {
        state.money = packet.state->money.value_or(0);
        state.equipmentValue = packet.state->equipmentValue.value_or(0);
        state.armor = packet.state->armor.value_or(0);
        state.helmet = packet.state->helmet.value_or(false);
    }

    for (const auto& w : packet.weapons) {
        state.weapons.push_back(CsgoWeaponState::from(w));
    }

    return state;
}

CsgoGsiStateManager::CsgoGsiStateManager():
    BaseLogWatcher(false, shared::nowUtc()) {

    _delegateId = CsgoGsiListener::singleton()->addGsiPacketListener([this](const CsgoGsiPacket& packet) {
        handlePacket(packet);
    });
}

CsgoGsiStateManager::~CsgoGsiStateManager() {
    CsgoGsiListener::singleton()->removeGsiPacketListener(_delegateId);
}

void CsgoGsiStateManager::handlePacket(const CsgoGsiPacket& packet) {
    std::lock_guard guard(_stateMutex);
    // We can reliably grab the local user's steam ID from the provider section of the gsi packet.
    if (!_localSteamId && packet.provider) {
        _localSteamId = packet.provider->steamId;
    }

    const auto tm = packet.timestamp();

    // First we need to check if we need to start a new match since the change from
    // no map -> map exists in the gsi packet doesn't trigger a "previously" diff.
    if (packet.map && !_matchState) {
        _matchState = CsgoMatchState{};
        _matchState->map = packet.map->name.value_or("Unknown");
        _matchState->mode = packet.map->mode.value_or("Unknown");

        if (!_matchState->isSupported()) {
            LOG_WARNING("Unsupported CSGO mode: " << _matchState->mode << std::endl);
            // Ideally we'd like to support all possible game modes.
            _matchState.reset();
            return;
        }

        if (packet.map->phase) {
            const auto phase = packet.map->phase.value();
            handleNewMatchPhase(tm, phase);
        }

        _tracked = {};
    }

    if (packet.previous && packet.previous->player && packet.previous->player->activity && packet.player && packet.player->activity) {
        // We need to check the 'map' too because the pause menu will also put CSGO into the 'menu' state.
        if (packet.player->activity.value() == "menu" && !packet.map) {
            notify(static_cast<int>(ECsgoGsiEvents::MainMenu), tm, nullptr, false);
        }
    }

    if (!_matchState) {
        return;
    }

    // Must be first so that we can know accurately which round we're on.
    bool freezeLoadout = false;
    if (packet.previous) {
        freezeLoadout = handleRoundDiff(tm, packet.previous->round, packet.round, packet);
    }

    // We need to detect the bomb plant separately as the planted phase doesn't
    // show up in 'previous' but rather in 'added' (which we don't track).
    if (packet.round && packet.round->bomb) {
        const auto bombState = packet.round->bomb.value();
        if (bombState == "planted") {
            auto roundIt = _matchState->rounds.find(_latestRound);
            if (roundIt != _matchState->rounds.end() && !roundIt->second.bombPlantTime) {
                roundIt->second.bombPlantTime = tm;
            }
        }
    }

    if (packet.previous) {
        handlePlayerDiff(tm, packet.previous->player, packet.player, packet);
    }

    // Copy player state into our own CsgoTrackedPlayerState structure.
    // We want to have this happen AFTER handlePlayerDiff because that function
    // will compare against this struct to get an accurate read on the number
    // of kills/deaths/assists that actually happened.
    updateTrackedUserState(tm, packet);

    if (freezeLoadout) {
        // This indicates the start of the play round.
        auto roundIt = _matchState->rounds.find(_latestRound);
        if (roundIt != _matchState->rounds.end()) {
            roundIt->second.playTime = tm;
            // Lock the user's loadout for the round at this point. We might miss stuff
            // if they buy after the round starts but that should be fine.
            if (packet.player && _localSteamId) {
                CsgoPlayerRoundState state = _tracked.round;
                state.steamId = _localSteamId.value();
                roundIt->second.players[_localSteamId.value()] = state;
            }
        }
    }
    
    // Must be last so that we capture all information BEFORE we end the match.
    if (packet.previous) {
        handleMapDiff(tm, packet.previous->map, packet.map, packet);
    }

    // There's one last match end edge case that we want to detect - when the user
    // quits in the middle of the match back to the menu. Similar to the main menu event,
    // we only care about this when the user goes back to the main menu and not when they
    // get to the pause menu in game. Thus 'map' must be null.
    if (_matchState && packet.player && packet.player->activity.value_or("") == "menu" && !packet.map) {
        endMatch(tm);
    }
}

void CsgoGsiStateManager::updateTrackedUserState(const shared::TimePoint& tm, const CsgoGsiPacket& packet) {
    if (!packet.player) {
        return;
    }

    if (!packet.player->steamId || packet.player->steamId.value() != _localSteamId.value()) {
        return;
    }

    if (packet.player->matchStats) {
        const auto& ms = packet.player->matchStats.value();
        if (ms.kills) {
            _tracked.match.kills = ms.kills.value();
        }

        if (ms.assists) {
            _tracked.match.assists = ms.assists.value();
        }

        if (ms.deaths) {
            _tracked.match.deaths = ms.deaths.value();
        }
    }

    if (packet.player->state) {
        const auto& state = packet.player->state.value();
        if (state.money) {
            _tracked.round.money = state.money.value();
        }

        if (state.equipmentValue) {
            _tracked.round.equipmentValue = state.equipmentValue.value();
        }

        if (state.armor) {
            _tracked.round.armor = state.armor.value();
        }

        if (state.helmet) {
            _tracked.round.helmet = state.helmet.value();
        }
    }

    if (packet.player->team) {
        _tracked.round.team = packet.player->team.value();
    }

    if (!packet.player->weapons.empty()) {
        _tracked.round.weapons.clear();
        for (const auto& w : packet.player->weapons) {
            _tracked.round.weapons.push_back(CsgoWeaponState::from(w));
        }
    }
}

void CsgoGsiStateManager::handleMapDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiMapPacket>& prev, const std::optional<CsgoGsiMapPacket>& now, const CsgoGsiPacket& packet) {
    if (!prev || !now || !_matchState) {
        return;
    }

    if (prev->phase && now->phase) {
        const auto phase = now->phase.value();
        handleNewMatchPhase(tm, phase);
    }
}

void CsgoGsiStateManager::handleNewMatchPhase(const shared::TimePoint& tm, const std::string& phase) {
    if (phase == "warmup") {
        // Are we actually able to transition BACK into warmup?
        // Not sure tbh.
        _matchState->warmupStart = tm;
    } else if (phase == "live") {
        startMatch(tm);
    } else if (phase == "gameover") {
        endMatch(tm);
    }
}

void CsgoGsiStateManager::handlePlayerDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiPlayerPacket>& prev, const std::optional<CsgoGsiPlayerPacket>& now, const CsgoGsiPacket& packet) {
    if (!prev || !now) {
        return;
    }

    // Only care about the local player for GSI.
    if (!now->steamId || now->steamId.value() != _localSteamId.value()) {
        return;
    }

    // If for some reason the previous player wasn't us, we want to ignore the differences as well.
    if (prev->steamId) {
        return;
    }

    auto it = _matchState->players.find(_localSteamId.value());
    if (it == _matchState->players.end()) {
        CsgoPlayerState newState = {};
        newState.steamId = _localSteamId.value();
        newState.name = now->name.value();
        _matchState->players[_localSteamId.value()] = newState;
        it = _matchState->players.find(_localSteamId.value());
    }

    // When the user first connects, the name might not be completely accurate as I think
    // the initial username is "unconnected". So whenever that changes, update our knowledge
    // of the user's name.
    if (prev->name && now->name) {
        it->second.name = now->name.value();
    }

    // Look for changes in global player stats and fit that into our trackage of global events.
    // Also use the changes in KDA to track the round kills array as well.
    if (prev->matchStats && now->matchStats) {
        auto roundIt = _matchState->rounds.find(_latestRound);
        if (roundIt != _matchState->rounds.end()) {
            if (prev->matchStats->kills && now->matchStats->kills && now->matchStats->kills.value() > _tracked.match.kills) {
                it->second.kills = now->matchStats->kills.value();

                const auto diff = now->matchStats->kills.value() - _tracked.match.kills;
                auto hs = (prev->state && prev->state->roundKillsHs && now->state && now->state->roundKillsHs) ? now->state->roundKillsHs.value() - prev->state->roundKillsHs.value() : 0;
                for (auto i = 0; i < diff; ++i) {
                    CsgoKillState kill = {};
                    kill.timestamp = tm;
                    kill.killer = _localSteamId;
                    kill.headshot = (hs-- > 0);

                    if (now->state) {
                        kill.flashed = now->state->flashed.value_or(0) >= 64;
                        kill.smoked = now->state->smoked.value_or(0) >= 64;
                    }

                    roundIt->second.kills.push_back(kill);
                }
            }

            if (prev->matchStats->assists && now->matchStats->assists && now->matchStats->assists.value() > _tracked.match.assists) {
                it->second.assists = now->matchStats->assists.value();

                const auto diff = now->matchStats->assists.value() - _tracked.match.assists;
                for (auto i = 0; i < diff; ++i) {
                    CsgoKillState kill = {};
                    kill.timestamp = tm;
                    kill.assisters = { _localSteamId.value() };
                    roundIt->second.kills.push_back(kill);
                }
            }

            if (prev->matchStats->deaths && now->matchStats->deaths && now->matchStats->deaths.value() > _tracked.match.deaths) {
                it->second.deaths = now->matchStats->deaths.value();

                const auto diff = now->matchStats->deaths.value() - _tracked.match.deaths;
                for (auto i = 0; i < diff; ++i) {
                    CsgoKillState kill = {};
                    kill.timestamp = tm;
                    kill.victim = _localSteamId;
                    roundIt->second.kills.push_back(kill);
                }
            }
        }

        if (prev->matchStats->mvps && now->matchStats->mvps) {
            it->second.mvps = now->matchStats->mvps.value();
        }

        if (prev->matchStats->score && now->matchStats->score) {
            it->second.score = now->matchStats->score.value();
        }
    }
}

bool CsgoGsiStateManager::handleRoundDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiRoundPacket>& prev, const std::optional<CsgoGsiRoundPacket>& now, const CsgoGsiPacket& packet) {
    if (!prev || !now) {
        return false;
    }

    if (prev->phase && now->phase) {
        if (now->phase.value() == "over" && now->winTeam) {
            // Generally a "win team" will show up along with a phase change
            // so this check should be safe (and fairly unique) to determine
            // when the round has been won and by whom. Note that we should not
            // increment what round we're on until the next buy round so that
            // any kills/deaths after time expires is properly attributed to the
            // player in *this* round.
            auto roundIt = _matchState->rounds.find(_latestRound);
            if (roundIt != _matchState->rounds.end()) {
                // Remember who, how, and when they won the round.
                roundIt->second.winningTeam = now->winTeam;
                if (packet.map && _latestRound < packet.map->roundWins.size()) {
                    const auto winMethod = packet.map->roundWins[_latestRound];
                    roundIt->second.roundWinMethod = winMethod;
                    if (winMethod == "ct_win_defuse" || winMethod == "t_win_bomb") {
                        roundIt->second.bombNextTime = tm;
                    }
                }
                roundIt->second.roundEndTime = tm;
            }
        } else if (now->phase.value() == "freezetime" && packet.map) {
            onStartNewRound(tm, packet.map->round.value_or(_latestRound), packet);
        } else if (now->phase.value() == "live") {
            // Return true to indicate that we want to freeze the user's loadout after we process it.
            return true;
        }
    }
    
    return false;
}

void CsgoGsiStateManager::startMatch(const shared::TimePoint& tm) {
    if (!_matchState) {
        return;
    }
    _matchState->start = tm;
    notify(static_cast<int>(ECsgoGsiEvents::MatchStart), tm, (void*)&_matchState.value(), false);
}

void CsgoGsiStateManager::endMatch(const shared::TimePoint& tm) {
    if (!_matchState) {
        return;
    }

    _matchState->end = tm;

    if (_matchState->start) {
        // We only send out a corresponding "match end" event if and only if
        // we sent out a corresponding "match start" event.
        notify(static_cast<int>(ECsgoGsiEvents::MatchEnd), tm, (void*)&_matchState.value(), false);
    }

    _latestRound = -1;
    _tracked = {};
    _matchState.reset();
}

void CsgoGsiStateManager::onStartNewRound(const shared::TimePoint& tm, int roundIdx, const CsgoGsiPacket& packet) {
    if (!_matchState) {
        return;
    }

    if (roundIdx <= _latestRound) {
        LOG_WARNING("Trying to start a new round with the same round index?" << std::endl);
        return;
    }

    // When we start a new round we just need to make sure we initialize the round state.
    CsgoRoundState newRound = {};
    newRound.roundNum = roundIdx;

    // Technically I'm not sure which one should be set since some modes don't have a "buy time"
    // so we probably need to detect the round phase or something instead.
    if (packet.map && (packet.map->mode.value_or("") == "casual" || packet.map->mode.value_or("") == "competitive")) {
        newRound.buyTime = tm;
    } else {
        newRound.playTime = tm;
    }

    // Initialize the current player if necessary (only if the player is the same as the local player).
    if (packet.player && _localSteamId && packet.player->steamId == _localSteamId) {
        newRound.players[_localSteamId.value()] = CsgoPlayerRoundState::from(packet.player.value());
    }

    _matchState->rounds[roundIdx] = newRound;
    _latestRound = std::max(roundIdx, _latestRound);
}

}