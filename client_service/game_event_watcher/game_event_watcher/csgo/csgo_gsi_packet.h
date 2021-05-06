#pragma once

#include "shared/json.h"
#include "shared/log/log.h"
#include "shared/time.h"

#include <memory>
#include <optional>

namespace game_event_watcher {

struct CsgoGsiTeamPacket {
    std::optional<int> score;
    std::optional<int> timeoutsRemaining;
};

struct CsgoGsiMapPacket {
    std::optional<int> currentSpectators;
    std::optional<std::string> mode;
    std::optional<std::string> name;
    std::optional<std::string> phase;
    std::optional<int> round;
    std::vector<std::string> roundWins;
    std::optional<CsgoGsiTeamPacket> teamCt;
    std::optional<CsgoGsiTeamPacket> teamT;
};

struct CsgoGsiPlayerMatchStatsPacket {
    std::optional<int> score;
    std::optional<int> mvps;
    std::optional<int> kills;
    std::optional<int> deaths;
    std::optional<int> assists;
};

struct CsgoGsiWeaponPacket {
    int weaponIdx;
    std::optional<std::string> name;
    std::optional<std::string> paintkit;
    std::optional<std::string> state;
    std::optional<std::string> type;
    std::optional<int> ammoReserve;
    std::optional<int> ammoClip;
    std::optional<int> ammoClipMax;
};

struct CsgoGsiPlayerStatePacket {
    std::optional<int> armor;
    std::optional<int> burning;
    std::optional<int> equipmentValue;
    std::optional<int> flashed;
    std::optional<int> smoked;
    std::optional<int> health;
    std::optional<int> money;
    std::optional<bool> helmet;
    std::optional<int> roundKills;
    std::optional<int> roundKillsHs;
    std::optional<bool> defuseKit;
};

struct CsgoGsiPlayerPacket {
    std::optional<std::string> activity;
    std::optional<std::string> name;
    std::optional<std::string> steamId;

    std::optional<CsgoGsiPlayerStatePacket> state;
    std::optional<CsgoGsiPlayerMatchStatsPacket> matchStats;
    std::optional<std::string> team;
    std::vector<CsgoGsiWeaponPacket> weapons;
};

struct CsgoGsiProviderPacket {
    std::optional<int64_t> appId;
    std::optional<std::string> name;
    std::optional<std::string> steamId;
    std::optional<int64_t> timestamp;
    std::optional<int> version;
};

struct CsgoGsiRoundPacket {
    // A couple of possible phases:
    //  - "freezetime": in defusal mode this is the buy time
    //  - "live": play time post buy-time
    //  - "over": when the round is finished
    std::optional<std::string> phase;
    std::optional<std::string> winTeam;
    // If set, this is either "planted" or "defused".
    // If not set, the bomb has not yet been planted.
    std::optional<std::string> bomb;
};

using CsgoGsiPacketPtr = std::unique_ptr<struct CsgoGsiPacket>;
struct CsgoGsiPacket {
    std::optional<CsgoGsiMapPacket> map;
    std::optional<CsgoGsiPlayerPacket> player;
    std::optional<CsgoGsiProviderPacket> provider;
    std::optional<CsgoGsiRoundPacket> round;
    CsgoGsiPacketPtr previous;

    shared::TimePoint timestamp() const;
};

}

std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiTeamPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiMapPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiWeaponPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiPlayerStatePacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiPlayerMatchStatsPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiPlayerPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiProviderPacket& p);
std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiRoundPacket& p);


namespace shared::json {

template<>
struct JsonConverter<game_event_watcher::CsgoGsiTeamPacket> {
    static game_event_watcher::CsgoGsiTeamPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiTeamPacket team;
        team.score = JsonConverter<decltype(team.score)>::fromKey(v, "score");
        team.timeoutsRemaining = JsonConverter<decltype(team.timeoutsRemaining)>::fromKey(v, "timeouts_remaining");
        return team;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiMapPacket> {
    static game_event_watcher::CsgoGsiMapPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiMapPacket map;
        map.currentSpectators = JsonConverter<decltype(map.currentSpectators)>::fromKey(v, "current_spectators");
        map.mode = JsonConverter<decltype(map.mode)>::fromKey(v, "mode");
        map.name = JsonConverter<decltype(map.name)>::fromKey(v, "name");
        map.phase = JsonConverter<decltype(map.phase)>::fromKey(v, "phase");
        map.round = JsonConverter<decltype(map.round)>::fromKey(v, "round");
        map.teamCt = JsonConverter<decltype(map.teamCt)>::fromKey(v, "team_ct");
        map.teamT = JsonConverter<decltype(map.teamCt)>::fromKey(v, "team_t");

        const auto roundWins = v.find("round_wins");
        if (roundWins != v.end()) {
            for (int i = 1;; ++i) {
                const auto it = roundWins->find(std::to_string(i));
                if (it == roundWins->end()) {
                    break;
                }
                map.roundWins.push_back(it->get<std::string>());
            }
        }

        return map;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiPlayerMatchStatsPacket> {
    static game_event_watcher::CsgoGsiPlayerMatchStatsPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiPlayerMatchStatsPacket stats;
        stats.score = JsonConverter<decltype(stats.score)>::fromKey(v, "score");
        stats.mvps = JsonConverter<decltype(stats.mvps)>::fromKey(v, "mvps");
        stats.kills = JsonConverter<decltype(stats.kills)>::fromKey(v, "kills");
        stats.deaths = JsonConverter<decltype(stats.deaths)>::fromKey(v, "deaths");
        stats.assists = JsonConverter<decltype(stats.assists)>::fromKey(v, "assists");
        return stats;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiWeaponPacket> {
    static game_event_watcher::CsgoGsiWeaponPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiWeaponPacket weapon;
        weapon.name = JsonConverter<decltype(weapon.name)>::fromKey(v, "name");
        weapon.paintkit = JsonConverter<decltype(weapon.paintkit)>::fromKey(v, "paintkit");
        weapon.state = JsonConverter<decltype(weapon.state)>::fromKey(v, "state");
        weapon.type = JsonConverter<decltype(weapon.type)>::fromKey(v, "type");
        weapon.ammoReserve = JsonConverter<decltype(weapon.ammoReserve)>::fromKey(v, "ammo_reserve");
        weapon.ammoClip = JsonConverter<decltype(weapon.ammoClip)>::fromKey(v, "ammo_clip");
        weapon.ammoClipMax = JsonConverter<decltype(weapon.ammoClipMax)>::fromKey(v, "ammo_clip_max");
        return weapon;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiPlayerStatePacket> {
    static game_event_watcher::CsgoGsiPlayerStatePacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiPlayerStatePacket pstate;
        pstate.armor = JsonConverter<decltype(pstate.armor)>::fromKey(v, "armor");
        pstate.burning = JsonConverter<decltype(pstate.burning)>::fromKey(v, "burning");
        pstate.equipmentValue = JsonConverter<decltype(pstate.equipmentValue)>::fromKey(v, "equip_value");
        pstate.flashed = JsonConverter<decltype(pstate.flashed)>::fromKey(v, "flashed");
        pstate.health = JsonConverter<decltype(pstate.health)>::fromKey(v, "health");
        pstate.helmet = JsonConverter<decltype(pstate.helmet)>::fromKey(v, "helmet");
        pstate.money = JsonConverter<decltype(pstate.money)>::fromKey(v, "money");
        pstate.roundKillsHs = JsonConverter<decltype(pstate.roundKillsHs)>::fromKey(v, "round_killhs");
        pstate.roundKills = JsonConverter<decltype(pstate.roundKills)>::fromKey(v, "round_kills");
        pstate.smoked = JsonConverter<decltype(pstate.smoked)>::fromKey(v, "smoked");
        pstate.defuseKit = JsonConverter<decltype(pstate.defuseKit)>::fromKey(v, "defusekit");
        return pstate;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiPlayerPacket> {
    static game_event_watcher::CsgoGsiPlayerPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiPlayerPacket player;
        player.activity = JsonConverter<decltype(player.activity)>::fromKey(v, "activity");
        player.name = JsonConverter<decltype(player.name)>::fromKey(v, "name");
        player.steamId = JsonConverter<decltype(player.steamId)>::fromKey(v, "steamid");
        player.team = JsonConverter<decltype(player.team)>::fromKey(v, "team");
        player.matchStats = JsonConverter<decltype(player.matchStats)>::fromKey(v, "match_stats");

        const auto weapons = v.find("weapons");
        if (weapons != v.end()) {
            for (int i = 0; i < 10; ++i) {
                std::ostringstream keyName;
                keyName << "weapon_" << i;

                const auto weapon = weapons->find(keyName.str());
                if (weapon == weapons->end()) {
                    continue;
                }

                auto parsedWeapon = JsonConverter<game_event_watcher::CsgoGsiWeaponPacket>::from(*weapon);
                parsedWeapon.weaponIdx = i;

                player.weapons.push_back(parsedWeapon);
            }
        }

        return player;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiProviderPacket> {
    static game_event_watcher::CsgoGsiProviderPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiProviderPacket provider;
        provider.appId = JsonConverter<decltype(provider.appId)>::fromKey(v, "appid");
        provider.name = JsonConverter<decltype(provider.name)>::fromKey(v, "name");
        provider.steamId = JsonConverter<decltype(provider.steamId)>::fromKey(v, "steamid");
        provider.timestamp = JsonConverter<decltype(provider.timestamp)>::fromKey(v, "timestamp");
        provider.version = JsonConverter<decltype(provider.version)>::fromKey(v, "version");
        return provider;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiRoundPacket> {
    static game_event_watcher::CsgoGsiRoundPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiRoundPacket round;
        round.phase = JsonConverter<decltype(round.phase)>::fromKey(v, "phase");
        round.bomb = JsonConverter<decltype(round.bomb)>::fromKey(v, "bomb");
        round.winTeam = JsonConverter<decltype(round.winTeam)>::fromKey(v, "win_team");
        return round;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoGsiPacket> {
    static game_event_watcher::CsgoGsiPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiPacket packet;
        packet.map = JsonConverter<decltype(packet.map)>::fromKey(v, "map");
        packet.player = JsonConverter<decltype(packet.player)>::fromKey(v, "player");
        packet.provider = JsonConverter<decltype(packet.provider)>::fromKey(v, "provider");
        packet.round = JsonConverter<decltype(packet.round)>::fromKey(v, "round");

        const auto previously = v.find("previously");
        if (previously != v.end()) {
            packet.previous = std::make_unique<game_event_watcher::CsgoGsiPacket>(
                JsonConverter<game_event_watcher::CsgoGsiPacket>::from(*previously)
            );
        } else {
            packet.previous = nullptr;
        }

        return packet;
    }
};

}