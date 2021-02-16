#include "game_event_watcher/wow/wow_log_watcher.h"
#include "shared/filesystem/utility.h"
#include "shared/timer.h"
#include <boost/algorithm/string.hpp>
#include <vector>

#define DEBUG_TIMING 0

using namespace std::literals::string_literals;
namespace fs = std::filesystem;
namespace game_event_watcher {
namespace {

bool parseRawCombatLogLine(const std::string& line, RawWoWCombatLog& log, const fs::path& logPath) {
    // General Format:
    // DATE TIME TOKENS
    std::vector<std::string> parts;
    boost::split(parts, line, boost::is_any_of(" "));
    if (parts.size() < 3) {
        return false;
    }

    const auto currentLocal = date::make_zoned(date::current_zone(), shared::filesystem::timeOfLastFileWrite(logPath)).get_local_time();
    std::ostringstream datetime;
    datetime << parts[0] << "/" << date::year_month_day{date::floor<date::days>(currentLocal)}.year() << " " << parts[1];

    const auto t = date::make_zoned(
        date::current_zone(),
        shared::strToLocalTime(datetime.str(), "%m/%d/%Y %T")
    );
    log.timestamp = t.get_sys_time();

    // Need to combine everything from the 3rd part onwards back
    // into one string since there could be spaces in the tokens
    // that aren't relevant to the initial DATE TIME TOKEN split.
    parts.erase(parts.begin(), parts.begin() + 2);
    const auto tokenPart = boost::join_if(parts, " ", [](const std::string& p) {
        return !boost::trim_copy(p).empty();
    });
    boost::split(log.log, tokenPart, boost::is_any_of(","));
    return true;
}

bool parseCombatLogStart(const RawWoWCombatLog& log, WoWCombatLogState& state) {
    if (log.log[0] != "COMBAT_LOG_VERSION") {
        return false;
    }

    state.combatLogVersion = log.log[1];
    state.advancedLog = (log.log[3] == "1");
    state.buildVersion = log.log[5];
    return true;
}

bool parseEncounterStart(const RawWoWCombatLog& log, WoWEncounterStart& encounter) {
    if (log.log[0] != "ENCOUNTER_START") {
        return false;
    }

    encounter.encounterId = std::stoi(log.log[1]);
    encounter.encounterName = log.log[2];
    encounter.difficulty = std::stoi(log.log[3]);
    encounter.numPlayers = std::stoi(log.log[4]);
    encounter.instanceId = std::stoi(log.log[5]);
    return true;    
}

bool parseEncounterEnd(const RawWoWCombatLog& log, WoWEncounterEnd& encounter) {
    if (log.log[0] != "ENCOUNTER_END") {
        return false;
    }

    encounter.encounterId = std::stoi(log.log[1]);
    encounter.encounterName = log.log[2];
    encounter.difficulty = std::stoi(log.log[3]);
    encounter.numPlayers = std::stoi(log.log[4]);
    encounter.success = (log.log[5] == "1");
    return true;    
}

bool parseChallengeModeStart(const RawWoWCombatLog& log, WoWChallengeModeStart& challenge) {
    if (log.log[0] != "CHALLENGE_MODE_START") {
        return false;
    }

    challenge.challengeName = log.log[1];
    challenge.instanceId = std::stoi(log.log[2]);
    challenge.keystoneLevel = std::stoi(log.log[4]);
    return true;    
}

bool parseChallengeModeEnd(const RawWoWCombatLog& log, WoWChallengeModeEnd& challenge) {
    if (log.log[0] != "CHALLENGE_MODE_END") {
        return false;
    }

    challenge.instanceId = std::stoi(log.log[1]);
    challenge.success = (log.log[2] == "1");
    challenge.keystoneLevel = std::stoi(log.log[3]);
    challenge.timeMs = static_cast<int64_t>(std::stoll(log.log[4]));
    return true;    
}

bool parseArenaStart(const RawWoWCombatLog& log, WoWArenaStart& arena) {
    if (log.log[0] != "ARENA_MATCH_START") {
        return false;
    }

    arena.instanceId = std::stoi(log.log[1]);
    arena.type = log.log[3];
    arena.localTeamId = std::stoi(log.log[4]);
    return true;
}

bool parseArenaEnd(const RawWoWCombatLog& log, WoWArenaEnd& arena) {
    if (log.log[0] != "ARENA_MATCH_END") {
        return false;
    }

    arena.winningTeamId = std::stoi(log.log[1]);
    arena.matchDurationSeconds = std::stoi(log.log[2]);
    arena.newRatings[0] = std::stoi(log.log[3]);
    arena.newRatings[1] = std::stoi(log.log[4]);
    return true;
}

bool parseCombatantInfo(const RawWoWCombatLog& log, WoWCombatantInfo& info) {
    if (log.log[0] != "COMBATANT_INFO") {
        return false;
    }
    info.guid = log.log[1];
    return true;
}

<<<<<<< HEAD
=======
bool parseZoneChange(const RawWoWCombatLog& log) {
    return log.log[0] == "ZONE_CHANGE";
}

>>>>>>> release
constexpr auto maxLogsToKeep = 10;

}

WoWLogWatcher::WoWLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold):
    BaseLogWatcher(useTimeChecks, timeThreshold) {

}

void WoWLogWatcher::loadFromExecutable(const fs::path& exePath) {
    const auto combatLogPath = exePath.parent_path() / fs::path("Logs") / fs::path("WoWCombatLog.txt");
    loadFromPath(combatLogPath);
}

void WoWLogWatcher::loadFromPath(const std::filesystem::path& combatLogPath, bool loop) {
    LOG_INFO("WoW Combat Log Path: " << combatLogPath.string() << " " << loop << std::endl);
    _logPath = combatLogPath;
    using std::placeholders::_1;
    _watcher = std::make_unique<LogWatcher>(combatLogPath, std::bind(&WoWLogWatcher::onCombatLogChange, this, _1), timeThreshold(), useTimeChecks(), false, loop);
    _watcher->disableBatching();
}

void WoWLogWatcher::wait() {
    _watcher->wait();
}

void WoWLogWatcher::onCombatLogChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
#if DEBUG_TIMING
        shared::Timer timer("Parse WoW Combat Log Line");
#endif

        RawWoWCombatLog log;
        log.logLine = _logLine++;
        if (!parseRawCombatLogLine(ln, log, _logPath)) {
            LOG_WARNING("Failed to parse WoW combat log line: " << ln << std::endl);
            continue;
        }

#if DEBUG_TIMING
        timer.tick("Raw Combat Log");
#endif

        bool parsed = false;

        if (!parsed) {
            WoWCombatLogState state;
            if (parseCombatLogStart(log, state)) {
                notify(static_cast<int>(EWoWLogEvents::CombatLogStart), log.timestamp, (void*)&state);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Raw Combat Log State");
#endif

        if (!parsed) {
            WoWEncounterStart encounter;
            if (parseEncounterStart(log, encounter)) {
                notify(static_cast<int>(EWoWLogEvents::EncounterStart), log.timestamp, (void*)&encounter);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Encounter Start");
#endif

        if (!parsed) {
            WoWEncounterEnd encounter;
            if (parseEncounterEnd(log, encounter)) {
                notify(static_cast<int>(EWoWLogEvents::EncounterEnd), log.timestamp, (void*)&encounter);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Encounter End");
#endif

        if (!parsed) {
            WoWChallengeModeStart challenge;
            if (parseChallengeModeStart(log, challenge)) {
                notify(static_cast<int>(EWoWLogEvents::ChallengeModeStart), log.timestamp, (void*)&challenge);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Challenge Start");
#endif

        if (!parsed) {
            WoWChallengeModeEnd challenge;
            if (parseChallengeModeEnd(log, challenge)) {
                notify(static_cast<int>(EWoWLogEvents::ChallengeModeEnd), log.timestamp, (void*)&challenge);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Challenge End");
#endif
        
        if (!parsed) {
            WoWArenaStart arena;
            if (parseArenaStart(log, arena)) {
                notify(static_cast<int>(EWoWLogEvents::ArenaStart), log.timestamp, (void*)&arena);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Arena Start");
#endif

        if (!parsed) {
            WoWArenaEnd arena;
            if (parseArenaEnd(log, arena)) {
                notify(static_cast<int>(EWoWLogEvents::ArenaEnd), log.timestamp, (void*)&arena);
                parsed = true;
            }
        }

#if DEBUG_TIMING
        timer.tick("Arena End");
#endif

        if (!parsed) {
            if (parseZoneChange(log)) {
                notify(static_cast<int>(EWoWLogEvents::ZoneChange), log.timestamp, nullptr);
            }
        }

        if (!parsed) {
            WoWCombatantInfo info;
            if (parseCombatantInfo(log, info)) {
                notify(static_cast<int>(EWoWLogEvents::CombatantInfo), log.timestamp, (void*)&info);
                parsed = true;
            } else {
                notify(static_cast<int>(EWoWLogEvents::FinishCombatantInfo), log.timestamp, nullptr, true, true);
            }
        }

#if DEBUG_TIMING
        timer.tick("Combatant Info");
#endif

        // We use this flag to determine whether or not we MUST send this log line to the server since it's
        // presumably important.
        log.parsed = parsed;

        // Every log line needs to get passed to the CombatLogLine event
        notify(static_cast<int>(EWoWLogEvents::CombatLogLine), log.timestamp, (void*)&log, true, true);

#if DEBUG_TIMING
        timer.tick("Generic Combat Log Line");
#endif
    }
}

void WoWLogWatcher::moveLogToBackup() {
    if (!fs::exists(_logPath)) {
        return;
    }

    // Force destruction fo the log watcher to release any handles we might have on this log file.
    _watcher.reset(nullptr);

    fs::rename(_logPath, shared::filesystem::generateTimestampBackupFileName(_logPath));

    // Remove older log files.
    auto dirIter = fs::directory_iterator(_logPath.parent_path());
    std::vector<fs::path> logPaths(fs::begin(dirIter), fs::end(dirIter));

    // Only manage the files that start with WoWCombatLog. We can just check that the
    // filename in the directory contains the stem of the given log path for this check
    // as the stem of _logPath will be WoWCombatLog. This helps us avoid having to
    // hardcode the "WoWCombatLog" string just in case it changes in the future.
    logPaths.erase(
        std::remove_if(logPaths.begin(), logPaths.end(), [this](const fs::path& pth){
            return pth.filename().native().find(_logPath.stem().native()) == std::filesystem::path::string_type::npos;
        }),
        logPaths.end()
    );

    shared::filesystem::pruneFilesystemPaths(logPaths, maxLogsToKeep);
}

nlohmann::json RawWoWCombatLog::toJson() const {
    auto parts = nlohmann::json::array();
    for (const auto& p : log) {
        parts.push_back(p);
    }

    return {
        {"timestamp", shared::timeToIso(timestamp)},
        // Uhhh...I made the mistake of naming it parts on the server instead of log. Oop.
        {"parts", parts },
        {"logLine", logLine }
    };
}


nlohmann::json WoWCombatLogState::toJson() const {
    return {
        {"combatLogVersion", combatLogVersion},
        {"advancedLog", advancedLog},
        {"buildVersion", buildVersion}
    };
}

nlohmann::json WoWCombatantInfo::toJson() const {
    return {
        { "guid", guid }
    };
}

nlohmann::json WoWEncounterStart::toJson() const {
    return {
        { "encounterId", encounterId },
        { "encounterName", encounterName },
        { "difficulty", difficulty },
        { "numPlayers", numPlayers },
        { "instanceId", instanceId },
    };
}

nlohmann::json WoWArenaStart::toJson() const {
    return {
        { "instanceId", instanceId },
        { "type", type },
        { "localTeamId", localTeamId }
    };
}

nlohmann::json WoWArenaEnd::toJson() const {
    nlohmann::json ratings = nlohmann::json::array();
    ratings.push_back(newRatings[0]);
    ratings.push_back(newRatings[1]);

    return {
        { "winningTeamId", winningTeamId },
        { "matchDurationSeconds", matchDurationSeconds },
        { "newRatings", ratings }
    };
}

nlohmann::json WoWEncounterEnd::toJson() const {
    return {
        { "encounterId", encounterId },
        { "encounterName", encounterName },
        { "difficulty", difficulty },
        { "numPlayers", numPlayers },
        { "success", success },
    };
}

nlohmann::json WoWChallengeModeStart::toJson() const {
    return {
        { "challengeName", challengeName },
        { "instanceId", instanceId },
        { "keystoneLevel", keystoneLevel }
    };
}

nlohmann::json WoWChallengeModeEnd::toJson() const {
    return {
        { "instanceId", instanceId },
        { "success", success },
        { "keystoneLevel", keystoneLevel },
        { "timeMs", timeMs }
    };
}

std::ostream& operator<<(std::ostream& os, const WoWEncounterStart& e) {
    os << "{"
       << "Id: " << e.encounterId 
       << ", Name:" << e.encounterName
       << ", Difficulty: " << e.difficulty
       << ", Players: " << e.numPlayers
       << ", Instance Id: " << e.instanceId
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WoWChallengeModeStart& e) {
    os << "{"
       << "Instance Id: " << e.instanceId 
       << ", Name:" << e.challengeName
       << ", Keystone: " << e.keystoneLevel
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WoWArenaStart& e) {
    os << "{"
       << "Instance Id: " << e.instanceId 
       << ", Type:" << e.type
       << ", Local Team Id: " << e.localTeamId
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WoWEncounterEnd& e) {
    os << "{"
       << "Id: " << e.encounterId 
       << ", Name:" << e.encounterName
       << ", Difficulty: " << e.difficulty
       << ", Players: " << e.numPlayers
       << ", Success: " << e.success
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WoWChallengeModeEnd& e) {
    os << "{"
       << "Instance Id: " << e.instanceId 
       << ", Success:" << e.success
       << ", Keystone: " << e.keystoneLevel
       << ", Time (ms): " << e.timeMs
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WoWArenaEnd& e) {
    os << "{"
       << "Winning Team: " << e.winningTeamId 
       << ", Match Duration (s):" << e.matchDurationSeconds
       << ", Ratings[0]: " << e.newRatings[0]
       << ", Ratings[1]: " << e.newRatings[1]
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WoWCombatantInfo& e) {
    os << "{Guid: " << e.guid << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<WoWCombatantInfo>& e) {
    os << "[";
    for (const auto& c : e) {
        os << "\t" << c << "," << std::endl;
    }
    os << "]";
    return os;
}

}