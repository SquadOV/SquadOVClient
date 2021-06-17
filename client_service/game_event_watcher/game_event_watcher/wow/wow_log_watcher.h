#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "shared/time.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>

namespace game_event_watcher {

enum class EWoWLogEvents {
    // This is for when the user begins the combat log with /combatlog
    CombatLogStart,
    // This event is emitted for every line of the combat log (this is needed
    // to send the log live to our servers).
    CombatLogLine,
    EncounterStart,
    EncounterEnd,
    ChallengeModeStart,
    ChallengeModeEnd,
    ArenaStart,
    ArenaEnd,
    CombatantInfo,
    // This event is fired whenever the line is not a COMBATANT_INFO. Should only
    // be handled after you start to exepct COMBATANT_INFO events.
    FinishCombatantInfo,
    ZoneChange
};

struct WoWCombatLogState {
    std::string combatLogVersion;
    bool advancedLog = false;
    std::string buildVersion;

    nlohmann::json toJson() const;
};

struct RawWoWCombatLog {
    shared::TimePoint timestamp;
    std::vector<std::string> log;
    std::string rawLog;
    int64_t logLine = 0;

    nlohmann::json toJson() const;
};

struct WoWEncounterStart {
    int encounterId = 0;
    std::string encounterName;
    int difficulty = 0;
    int numPlayers = 0;
    int instanceId = 0;

    nlohmann::json toJson() const;
};

struct WoWChallengeModeStart {
    std::string challengeName;
    int instanceId = 0;
    int keystoneLevel = 0;

    nlohmann::json toJson() const;
};

struct WoWEncounterEnd {
    int encounterId = 0;
    std::string encounterName;
    int difficulty = 0;
    int numPlayers = 0;
    bool success = false;

    nlohmann::json toJson() const;
};

struct WoWChallengeModeEnd {
    int instanceId = 0;
    bool success = false;
    int keystoneLevel = 0;
    int64_t timeMs = 0;

    nlohmann::json toJson() const;
};

struct WoWArenaStart {
    int instanceId = 0;
    std::string type;
    int localTeamId = 0;

    nlohmann::json toJson() const;
};

struct WoWArenaEnd {
    int winningTeamId = 0;
    int matchDurationSeconds = 0;
    int newRatings[2];

    nlohmann::json toJson() const;
};

struct WoWCombatantInfo {
    std::string guid;

    nlohmann::json toJson() const;
};

struct WoWZoneChange {
    std::string zoneName;
    int instanceId = 0;
};

class WoWLogWatcher : public BaseLogWatcher {
public:
    WoWLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold);
    ~WoWLogWatcher();
    void loadFromExecutable(const std::filesystem::path& exePath);
    void loadFromPath(const std::filesystem::path& logPath, bool loop = true, bool legacy = false);
    void moveLogToBackup();

    bool legacy() const { return _legacy; }

    void wait();
private:
    void onCombatLogChange(const LogLinesDelta& lines);

    std::filesystem::path _logPath;
    LogWatcherPtr _watcher;
    int64_t _logLine = 0;
    bool _running = true;
    bool _legacy = false;
    std::thread _timestampLogThread;
    shared::TimePoint _lastLogTime = shared::zeroTime();
};

using WoWLogWatcherPtr = std::unique_ptr<WoWLogWatcher>;

std::ostream& operator<<(std::ostream& os, const WoWEncounterStart& e);
std::ostream& operator<<(std::ostream& os, const WoWChallengeModeStart& e);
std::ostream& operator<<(std::ostream& os, const WoWArenaStart& e);
std::ostream& operator<<(std::ostream& os, const WoWEncounterEnd& e);
std::ostream& operator<<(std::ostream& os, const WoWChallengeModeEnd& e);
std::ostream& operator<<(std::ostream& os, const WoWArenaEnd& e);
std::ostream& operator<<(std::ostream& os, const WoWCombatantInfo& e);
std::ostream& operator<<(std::ostream& os, const std::vector<WoWCombatantInfo>& e);

}