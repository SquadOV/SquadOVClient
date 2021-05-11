#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/csgo/csgo_gsi_packet.h"
#include "shared/time.h"
#include "shared/json.h"

#include <unordered_map>
#include <vector>

namespace game_event_watcher {

struct CsgoWeaponState {
    std::string name;
    std::string type;
    std::string paintkit;

    static CsgoWeaponState from(const CsgoGsiWeaponPacket& packet);
};

struct CsgoPlayerRoundState {
    std::string steamId;
    std::string team;
    std::vector<CsgoWeaponState> weapons;
    int money = 0;
    int equipmentValue = 0;
    int armor = 0;
    bool helmet = false;

    static CsgoPlayerRoundState from(const CsgoGsiPlayerPacket& packet);
};

struct CsgoKillState {
    shared::TimePoint timestamp;
    std::optional<std::string> killer;
    std::optional<std::string> victim;
    std::vector<std::string> assisters;

    std::optional<std::string> weapon;
    std::optional<bool> headshot;
    std::optional<bool> flashed;
    std::optional<bool> smoked;
};

struct CsgoRoundState {
    int roundNum = 0;

    std::optional<std::string> winningTeam;
    std::optional<std::string> roundWinMethod;

    std::unordered_map<std::string, CsgoPlayerRoundState> players;
    std::vector<CsgoKillState> kills;

    std::optional<shared::TimePoint> buyTime;
    std::optional<shared::TimePoint> playTime;
    std::optional<shared::TimePoint> bombPlantTime;
    // Defusal and or explosion.
    std::optional<shared::TimePoint> bombNextTime;
    std::optional<shared::TimePoint> roundEndTime;
};

struct CsgoPlayerState {
    int kills = 0;
    int deaths = 0;
    int assists = 0;
    int mvps = 0;
    int score = 0;
    std::string name;
    std::string steamId;
};

struct CsgoMatchState {
    std::string map;
    std::string mode;
    std::optional<std::string> winner;
    std::optional<std::string> connectedServer;

    std::optional<shared::TimePoint> warmupStart;
    std::optional<shared::TimePoint> start;
    std::optional<shared::TimePoint> end;

    std::unordered_map<int, CsgoRoundState> rounds;
    std::unordered_map<std::string, CsgoPlayerState> players;

    bool isSupported() const;
};

struct CsgoTrackedPlayerState {
    CsgoPlayerState match;
    CsgoPlayerRoundState round;
};

enum class ECsgoGsiEvents {
    MatchStart,
    MatchEnd
};

class CsgoGsiStateManager: public BaseLogWatcher {
public:
    CsgoGsiStateManager();
    ~CsgoGsiStateManager();

    void handlePacket(const CsgoGsiPacket& packet);

    std::string localSteamId() const { return _localSteamId.value(); }

private:
    void handleMapDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiMapPacket>& prev, const std::optional<CsgoGsiMapPacket>& now, const CsgoGsiPacket& packet);
    void handlePlayerDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiPlayerPacket>& prev, const std::optional<CsgoGsiPlayerPacket>& now, const CsgoGsiPacket& packet);
    bool handleRoundDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiRoundPacket>& prev, const std::optional<CsgoGsiRoundPacket>& now, const CsgoGsiPacket& packet);

    void startMatch(const shared::TimePoint& tm);
    void endMatch(const shared::TimePoint& tm);

    void handleNewMatchPhase(const shared::TimePoint& tm, const std::string& phase);
    void onStartNewRound(const shared::TimePoint& tm, int roundIdx, const CsgoGsiPacket& packet);

    std::optional<std::string> _localSteamId;
    std::optional<CsgoMatchState> _matchState;
    std::mutex _stateMutex;
    int _latestRound = -1;
    int64_t _delegateId = 0;

    CsgoTrackedPlayerState _tracked;
    void updateTrackedUserState(const shared::TimePoint& tm, const CsgoGsiPacket& packet);
};

}

namespace shared::json {

template<>
struct JsonConverter<game_event_watcher::CsgoWeaponState> {
    static nlohmann::json to(const game_event_watcher::CsgoWeaponState& v) {
        const nlohmann::json data = {
            { "name", v.name },
            { "type", v.type },
            { "paintkit", v.paintkit }
        };
        return data;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoPlayerRoundState> {
    static nlohmann::json to(const game_event_watcher::CsgoPlayerRoundState& v) {
        const nlohmann::json data = {
            { "steamId", v.steamId },
            { "team", v.team },
            { "money", v.money },
            { "equipmentValue", v.equipmentValue },
            { "armor", v.armor },
            { "helmet", v.helmet },
            { "weapons", JsonConverter<decltype(v.weapons)>::to(v.weapons)}
        };
        return data;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoKillState> {
    static nlohmann::json to(const game_event_watcher::CsgoKillState& v) {
        const nlohmann::json data = {
            { "timestamp", JsonConverter<decltype(v.timestamp)>::to(v.timestamp) },
            { "killer", JsonConverter<decltype(v.killer)>::to(v.killer) },
            { "victim", JsonConverter<decltype(v.victim)>::to(v.victim) },
            { "assisters", JsonConverter<decltype(v.assisters)>::to(v.assisters) },
            { "weapon", JsonConverter<decltype(v.weapon)>::to(v.weapon) },
            { "headshot", JsonConverter<decltype(v.headshot)>::to(v.headshot) },
            { "flashed", JsonConverter<decltype(v.flashed)>::to(v.flashed) },
            { "smoked", JsonConverter<decltype(v.smoked)>::to(v.smoked) }
        };

        return data;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoRoundState> {
    static nlohmann::json to(const game_event_watcher::CsgoRoundState& v) {
        const nlohmann::json data = {
            { "roundNum", v.roundNum },
            { "winningTeam", JsonConverter<decltype(v.winningTeam)>::to(v.winningTeam) },
            { "roundWinMethod", JsonConverter<decltype(v.roundWinMethod)>::to(v.roundWinMethod) },
            { "buyTime", JsonConverter<decltype(v.buyTime)>::to(v.buyTime) },
            { "playTime", JsonConverter<decltype(v.playTime)>::to(v.playTime) },
            { "bombPlantTime", JsonConverter<decltype(v.bombPlantTime)>::to(v.bombPlantTime) },
            { "bombNextTime", JsonConverter<decltype(v.bombNextTime)>::to(v.bombNextTime) },
            { "roundEndTime", JsonConverter<decltype(v.roundEndTime)>::to(v.roundEndTime) },
            { "players", JsonConverter<decltype(v.players)>::to(v.players) },
            { "kills", JsonConverter<decltype(v.kills)>::to(v.kills) }
        };
        return data;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoPlayerState> {
    static nlohmann::json to(const game_event_watcher::CsgoPlayerState& v) {
        const nlohmann::json data = {
            { "kills", v.kills },
            { "deaths", v.deaths },
            { "assists", v.assists },
            { "mvps", v.mvps },
            { "score", v.score },
            { "name", v.name },
            { "steamId", v.steamId }
        };
        return data;
    }
};

template<>
struct JsonConverter<game_event_watcher::CsgoMatchState> {
    static nlohmann::json to(const game_event_watcher::CsgoMatchState& v) {
        const nlohmann::json data = {
            { "map", v.map },
            { "mode", v.mode },
            { "winner", JsonConverter<decltype(v.winner)>::to(v.winner) },
            { "connectedServer", JsonConverter<decltype(v.connectedServer)>::to(v.connectedServer) },
            { "warmupStart", JsonConverter<decltype(v.warmupStart)>::to(v.warmupStart) },
            { "start", JsonConverter<decltype(v.start)>::to(v.start) },
            { "end", JsonConverter<decltype(v.end)>::to(v.end) },
            { "rounds", JsonConverter<std::vector<game_event_watcher::CsgoRoundState>>::to(v.rounds) },
            { "players", JsonConverter<decltype(v.players)>::to(v.players) }
        };
        return data;
    }
};

}