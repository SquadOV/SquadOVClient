#include "valorant/valorant_match_details.h"

#include <iostream>

namespace service::valorant {

void ValorantMatchRound::setLoadouts(std::vector<ValorantMatchLoadoutPtr>&& loadouts) {
    _playerLoadouts.clear();
    for (auto& l : loadouts) {
        _playerLoadouts[l->puuid()] = std::move(l);
    }
}

void ValorantMatchDetails::parseFromJson(const std::string& json) {
    _rawApiData = json;
    
    const auto parsedJson = nlohmann::json::parse(json);
    
    const auto matchInfo = parsedJson["matchInfo"];
    parseMatchInfoJson(matchInfo);

    // This must be called BEFORE players since players will be the one
    // to determine which team they're in.
    const auto teams = parsedJson["teams"];
    parseTeamsJson(teams);

    // Must parse players before kills and round results
    // so we know who we can attribute kills/deaths/damage to.
    const auto players = parsedJson["players"];
    parsePlayersJson(players);

    // Must be parsed before kills so that the kills can
    // insert themselves into the appropriate round.
    const auto roundResults = parsedJson["roundResults"];
    parseRoundsJson(roundResults);

    const auto kills = parsedJson["kills"];
    parseKillsJson(kills);

}

void ValorantMatchDetails::parseMatchInfoJson(const nlohmann::json& obj) {
    _map = obj["mapId"].get<std::string>();
    _gameMode = obj["gameMode"].get<std::string>();
    _provisioningFlowID = obj["provisioningFlowID"].get<std::string>();
    _isRanked = obj["isRanked"].get<bool>();
    _gameVersion = obj["gameVersion"].get<std::string>();
    _startTime = obj["gameStartMillis"].get<long long>() / 1000;
    _matchId = obj["matchId"].get<std::string>();
}

void ValorantMatchDetails::parsePlayersJson(const nlohmann::json& obj) {
    for (const auto& pdata : obj) {
        const auto puuid = pdata["subject"].get<std::string>();
        const auto stats = pdata["stats"];

        ValorantMatchPlayerPtr player = std::make_unique<ValorantMatchPlayer>();
        player->_puuid = puuid;
        player->_characterId = pdata["characterId"].get<std::string>();
        player->_stats.totalCombatScore = stats["score"].get<int>();
        player->_stats.roundsPlayed = stats["roundsPlayed"].get<int>();
        player->_stats.kills = stats["kills"].get<int>();
        player->_stats.deaths = stats["deaths"].get<int>();
        player->_stats.assists = stats["assists"].get<int>();

        if (_isRanked) {
            player->_competitiveTier = pdata["competitiveTier"].get<int>();
        }

        const auto teamId = pdata["teamId"].get<std::string>();
        auto* team = _teams[teamId].get();
        team->_players.push_back(player.get());
        player->_team = team;

        _players[puuid] = std::move(player);
    }
}

void ValorantMatchDetails::parseTeamsJson(const nlohmann::json& obj) {
    for (const auto& tdata : obj) {
        ValorantMatchTeamPtr team = std::make_unique<ValorantMatchTeam>();
        team->_teamId = tdata["teamId"].get<std::string>();
        team->_won = tdata["won"].get<bool>();
        team->_roundsPlayed = tdata["roundsPlayed"].get<int>();
        team->_roundsWon = tdata["roundsWon"].get<int>();
        _teams[team->_teamId] = std::move(team);
    }
}

void ValorantMatchDetails::parseRoundsJson(const nlohmann::json& obj) {
    for (const auto& rdata : obj) {
        ValorantMatchRoundPtr round = std::make_unique<ValorantMatchRound>();
        round->_roundNum = rdata["roundNum"].get<int>();

        round->_plantRoundTime = rdata["plantRoundTime"].get<int>();
        if (!!round->_plantRoundTime) {
            round->_planter = _players[rdata["bombPlanter"].get<std::string>()].get();
        }

        round->_defuseRoundTime = rdata["defuseRoundTime"].get<int>();
        if (!!round->_defuseRoundTime) {
            round->_defuser = _players[rdata["bombDefuser"].get<std::string>()].get();
        }

        round->_roundWinner = rdata["winningTeam"].get<std::string>();

        // The only thing we have to pull out of here is the damage that player did this round 
        // and their combat score for the round since all the rest of the information is parsed elsewhere.
        for (const auto& statData : rdata["playerStats"]) {
            const auto puuid = statData["subject"].get<std::string>();
            auto player = _players[puuid].get();

            const auto combatScore = statData["score"].get<int>();
            round->_roundCombatScore[puuid] = combatScore;

            for (const auto& dmgData : statData["damage"]) {
                ValorantMatchDamagePtr dmg = std::make_unique<ValorantMatchDamage>();
                dmg->_instigator = player;
                dmg->_receiver = _players[dmgData["receiver"].get<std::string>()].get();
                dmg->_damage = dmgData["damage"].get<int>();
                dmg->_legshots = dmgData["legshots"].get<int>();
                dmg->_bodyshots = dmgData["bodyshots"].get<int>();
                dmg->_headshots = dmgData["headshots"].get<int>();
                dmg->_roundNum = round->_roundNum;

                player->_damageDone.push_back(dmg.get());
                round->_damage.push_back(dmg.get());
                _damage.emplace_back(std::move(dmg));
            }
        }

        for (const auto& econData : rdata["playerEconomies"]) {
            ValorantMatchLoadoutPtr loadout = std::make_unique<ValorantMatchLoadout>();
            const auto puuid = econData["subject"].get<std::string>();
            loadout->_armor = econData["armor"].get<std::string>();
            loadout->_weapon = econData["weapon"].get<std::string>();
            loadout->_remainingMoney = econData["remaining"].get<int>();
            loadout->_loadoutValue = econData["loadoutValue"].get<int>();
            loadout->_spentMoney = econData["spent"].get<int>();
            loadout->_owner = _players[puuid].get();
            loadout->_roundNum = round->_roundNum;
            round->_playerLoadouts[puuid] = std::move(loadout);
        }

        _rounds.emplace_back(std::move(round));
    }
}

void ValorantMatchDetails::parseKillsJson(const nlohmann::json& obj) {
    for (const auto& kdata : obj) {
        ValorantMatchKillPtr kill = std::make_unique<ValorantMatchKill>();
        kill->_roundTime = kdata["roundTime"].get<int>();
        kill->_round = kdata["round"].get<int>();
        kill->_damageType = kdata["finishingDamage"]["damageType"].get<std::string>();
        kill->_damageItem = kdata["finishingDamage"]["damageItem"].get<std::string>();
        kill->_killSecondaryFire = kdata["finishingDamage"]["isSecondaryFireMode"].get<bool>();

        auto killer = _players[kdata["killer"].get<std::string>()].get();
        kill->_killer = killer;
        killer->_kills.push_back(kill.get());

        auto victim = _players[kdata["victim"].get<std::string>()].get();
        kill->_victim = victim;
        victim->_deaths.push_back(kill.get());

        _rounds[kill->_round]->_kills.push_back(kill.get());
        _kills.emplace_back(std::move(kill));
    }
}

void ValorantMatchDetails::mergeWithApi(ValorantMatchDetails* other) {
    _gameMode = other->_gameMode;
    _map = other->_map;
    _isRanked = other->_isRanked;
    _provisioningFlowID = other->_provisioningFlowID;
    _gameVersion = other->_gameVersion;
    _startTime = other->_startTime;
    _matchId = other->_matchId;
    _rawApiData = other->_rawApiData;

    _players = std::move(other->_players);
    _teams = std::move(other->_teams);

    // The only thing we need to pay attention to are the round
    // objects to make sure we retain the round timestamps if any.
    {
        auto oldRounds = std::move(_rounds);
        _rounds = std::move(other->_rounds);

        // There's a match so we can transfer over the data.
        if (oldRounds.size() != 0 && oldRounds.size() == _rounds.size()) {
            for (size_t i = 0; i < _rounds.size(); ++i) {
                _rounds[i]->_startBuyTime = oldRounds[i]->_startBuyTime;
                _rounds[i]->_startPlayTime = oldRounds[i]->_startPlayTime;
            }
        } else {
            std::cerr << "Mismatch in number of rounds between locally collected data and API." << std::endl;
        }
    }
    _kills = std::move(other->_kills);
    _damage = std::move(other->_damage);
}

void ValorantMatchDetails::nextRoundStateWithTimestamp(const shared::TimePoint& tm, shared::valorant::EValorantRoundState state) {
    // If the input state is "Buy" then we know that we
    // want to go to a new round. The input state is "Play" then
    // we should just be transferring the latest round to a new state.
    if (state == shared::valorant::EValorantRoundState::Buy) {
        // Only go to the buy state if we're currently *not* in a buy round or
        // if there are no rounds. This helps us handle the situation in a custom
        // game restart where the BUY event will be triggered BUT when the game starts
        // normally the BUY event IS NOT triggered. Yikes.
        if (_rounds.size() == 0 || _rounds.back()->currentRoundState() != shared::valorant::EValorantRoundState::Buy) {
            auto newRound = std::make_unique<ValorantMatchRound>();
            newRound->_startBuyTime = tm;
            newRound->_roundNum = static_cast<int>(_rounds.size() + 1);
            _rounds.emplace_back(std::move(newRound));
        }
    } else if (_rounds.size() > 0) {
        ValorantMatchRound* latestRound = _rounds.back().get();
        latestRound->_startPlayTime = tm;
    } else {
        // Technically there's something that's gone horribly wrong here.
    }
}

void ValorantMatchDetails::setPlayers(std::vector<ValorantMatchPlayerPtr>& players) {
    _players.clear();
    for (auto& p : players) {
        _players[p->puuid()] = std::move(p);
    }
}

void ValorantMatchDetails::setTeams(std::vector<ValorantMatchTeamPtr>& teams) {
    _teams.clear();
    for (auto& t : teams) {
        _teams[t->teamId()] = std::move(t);
    }
}

void ValorantMatchDetails::setRounds(std::vector<ValorantMatchRoundPtr>&& r) {
    _rounds = std::move(r);
}

void ValorantMatchDetails::setKills(std::vector<ValorantMatchKillPtr>&& k) {
    _kills = std::move(k);
}

void ValorantMatchDetails::setDamage(std::vector<ValorantMatchDamagePtr>&& d) {
    _damage = std::move(d);
}

shared::valorant::EValorantRoundState ValorantMatchRound::currentRoundState() const {
    if (shared::isTimeValid(_startPlayTime)) {
        return shared::valorant::EValorantRoundState::Play;
    } else if (shared::isTimeValid(_startBuyTime)) {
        return shared::valorant::EValorantRoundState::Buy;
    }
    return shared::valorant::EValorantRoundState::Unknown;
}

std::ostream& operator<<(std::ostream& out, const ValorantMatchDetails& details) {
    out << "MATCH: " << details.matchId() << std::endl
        << "\tGame Mode: " << details.gameMode() << std::endl
        << "\tMap: " << details.map() << std::endl
        << "\tIs Ranked: " << details.isRanked() << std::endl
        << "\tProvisioning Flow ID: " << details.provisioningFlowID() << std::endl
        << "\tGame Version: " << details.gameVersion() << std::endl
        << "\tStart Time: " << details.startTime() << std::endl;

    out << "TOTAL PLAYERS: " << details.players().size() << std::endl;
    for (const auto& [puuid, player] : details.players()) {
        out << *player;
    }

    out << "TOTAL TEAMS: " << details.teams().size() << std::endl;
    for (const auto& [teamId, team] : details.teams()) {
        out << *team;
    }

    out << "TOTAL ROUNDS: " << details.rounds().size() << std::endl;
    for (const auto& round : details.rounds()) {
        out << *round;
    }

    out << "TOTAL KILLS: " << details.kills().size() << std::endl;
    for (const auto& kills : details.kills()) {
        out << *kills;
    }

    out << "TOTAL DAMAGE: " << details.damage().size() << std::endl;
    for (const auto& dmg : details.damage()) {
        out << *dmg;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const ValorantMatchDamage& dmg) {
    out << "DAMAGE FROM " << dmg.instigatorPuuid() << " to " << dmg.receiverPuuid() << std::endl
        << "\tRound: " << dmg.roundNum() << std::endl
        << "\tDamage: " << dmg.damage() << std::endl
        << "\tHeadshots: " << dmg.headshots() << std::endl
        << "\tBodyshots: " << dmg.bodyshots() << std::endl
        << "\tLegshots: " << dmg.legshots() << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const ValorantMatchKill& kill) {
    out << "KILL BY " << kill.killerPuuid() << " on " << kill.victimPuuid() << std::endl
        << "\tRound: " << kill.round() << " at " << kill.roundTime() << std::endl
        << "\tUsing: " << kill.damageType() << " - " << kill.damageItem() << "[Secondary: " << kill.killSecondaryFire() << "]" << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const ValorantMatchRound& round) {
    out << "ROUND " << round.roundNum() << std::endl;
    out << "\tWinner: " << round.teamWinner() << std::endl;
    if (round.didPlant()) {
        out << "\tBOMB PLANTED AT " << round.plantRoundTime() << " by " << round.planterPuuid() << std::endl;
    }

    if (round.didDefuse()) {
        out << "\tBOMB DEFUSED AT " << round.defuseRoundTime() << " by " << round.defuserPuuid() << std::endl;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const ValorantMatchPlayer& player) {
    out << "PLAYER: " << player.puuid() << std::endl
        << "\tTeam: " << player.teamId() << std::endl
        << "\tAgent: " << player.agentId() << std::endl
        << "\tTier: " << player.competitiveTier() << std::endl
        << "\tCombat Score: " << player.stats().totalCombatScore << std::endl
        << "\tRounds Played: " << player.stats().roundsPlayed << std::endl
        << "\tKills: " << player.stats().kills << std::endl
        << "\tDeaths: " << player.stats().deaths << std::endl
        << "\tAssists: " << player.stats().assists << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const ValorantMatchTeam& team) {
    out << "TEAM: " << team.teamId() << std::endl
        << "\tWon: " << team.won() << std::endl
        << "\tRounds: " << team.roundsWon() << " / " << team.roundsPlayed() << std::endl;
    return out;
}

}