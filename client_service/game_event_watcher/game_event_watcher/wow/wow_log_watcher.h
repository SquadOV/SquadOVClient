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
    CombatantInfo,
    // This event is fired whenever the line is not a COMBATANT_INFO. Should only
    // be handled after you start to exepct COMBATANT_INFO events.
    FinishCombatantInfo
};

struct WoWCombatLogState {
    std::string combatLogVersion;
    bool advancedLog;
    std::string buildVersion;

    nlohmann::json toJson() const;
};

struct RawWoWCombatLog {
    shared::TimePoint timestamp;
    std::vector<std::string> log;

    nlohmann::json toJson() const;
};

struct WoWEncounterStart {
    int encounterId;
    std::string encounterName;
    int difficulty;
    int numPlayers;
    int instanceId;

    nlohmann::json toJson() const;
};

struct WoWChallengeModeStart {
    std::string challengeName;
    int instanceId;
    int keystoneLevel;

    nlohmann::json toJson() const;
};

struct WoWEncounterEnd {
    int encounterId;
    std::string encounterName;
    int difficulty;
    int numPlayers;
    bool success;

    nlohmann::json toJson() const;
};

struct WoWChallengeModeEnd {
    int instanceId;
    bool success;
    int keystoneLevel;
    int64_t timeMs;

    nlohmann::json toJson() const;
};

struct WoWCombatantInfo {
    std::string guid;

    nlohmann::json toJson() const;
};

class WoWLogWatcher : public BaseLogWatcher {
public:
    WoWLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold);
    void loadFromExecutable(const std::filesystem::path& exePath);
    void loadFromPath(const std::filesystem::path& logPath);

private:
    void onCombatLogChange(const LogLinesDelta& lines);

    std::filesystem::path _logPath;
    LogWatcherPtr _watcher;
};

using WoWLogWatcherPtr = std::unique_ptr<WoWLogWatcher>;

std::ostream& operator<<(std::ostream& os, const WoWEncounterStart& e);
std::ostream& operator<<(std::ostream& os, const WoWChallengeModeStart& e);
std::ostream& operator<<(std::ostream& os, const WoWEncounterEnd& e);
std::ostream& operator<<(std::ostream& os, const WoWChallengeModeEnd& e);
std::ostream& operator<<(std::ostream& os, const WoWCombatantInfo& e);
std::ostream& operator<<(std::ostream& os, const std::vector<WoWCombatantInfo>& e);

}