#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/csgo/csgo_gsi_packet.h"
#include "shared/time.h"

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
    std::unordered_map<int, CsgoWeaponState> weapons;
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

enum class ECsgoGsiEvents {
    MatchStart,
    MatchEnd
};

class CsgoGsiStateManager: public BaseLogWatcher {
public:
    CsgoGsiStateManager();
    ~CsgoGsiStateManager();

    void handlePacket(const CsgoGsiPacket& packet);
private:
    void handleMapDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiMapPacket>& prev, const std::optional<CsgoGsiMapPacket>& now, const CsgoGsiPacket& packet);
    void handlePlayerDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiPlayerPacket>& prev, const std::optional<CsgoGsiPlayerPacket>& now, const CsgoGsiPacket& packet);
    void handleRoundDiff(const shared::TimePoint& tm, const std::optional<CsgoGsiRoundPacket>& prev, const std::optional<CsgoGsiRoundPacket>& now, const CsgoGsiPacket& packet);

    void startMatch(const shared::TimePoint& tm);
    void endMatch(const shared::TimePoint& tm);

    void handleNewMatchPhase(const shared::TimePoint& tm, const std::string& phase);
    void onStartNewRound(const shared::TimePoint& tm, int roundIdx, const CsgoGsiPacket& packet);

    std::optional<std::string> _localSteamId;
    std::optional<CsgoMatchState> _matchState;
    int _latestRound = -1;
    int64_t _delegateId = 0;
};

}