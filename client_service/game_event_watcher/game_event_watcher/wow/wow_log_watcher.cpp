#include "game_event_watcher/wow/wow_log_watcher.h"
#include "shared/filesystem/utility.h"
#include "shared/timer.h"
#include "shared/errors/error.h"
#include "shared/version.h"
#include "shared/time/ntp_client.h"
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <vector>

using namespace std::literals::string_literals;
namespace fs = std::filesystem;
namespace game_event_watcher {
namespace {

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

bool parseZoneChange(const RawWoWCombatLog& log, WoWZoneChange& zone) {
    if (log.log[0] != "ZONE_CHANGE") {
        return false;
    }

    zone.instanceId = std::stoi(log.log[1]);
    zone.zoneName = log.log[2];
    return true;
}

bool parseSpellCastSuccess(const RawWoWCombatLog& log, WowSpellCastSuccess& cast) {
    if (log.log[0] != "SPELL_CAST_SUCCESS") {
        return false;
    }

    // Only parse the source and only if it's a player.
    // If it's not a player then we can possibly fuck up in parsing the name since the simple
    // split on commas won't work in that case.
    if (log.log[1].rfind("Player-") != 0) {
        return false;
    }

    cast.src.guid = log.log[1];
    cast.src.name = log.log[2];
    cast.src.flags = std::stoll(log.log[3], nullptr, 16);
    cast.src.raidFlags = std::stoll(log.log[4], nullptr, 16);
    return true;
}

constexpr auto maxLogsToKeep = 10;

}

bool parseRawCombatLogLine(const std::string& line, RawWoWCombatLog& log, const fs::path& logPath) {
    // General Format:
    // DATE TIME TOKENS
    std::vector<std::string> parts;
    boost::split(parts, line, boost::is_any_of(" "));
    if (parts.size() < 3) {
        return false;
    }

    // We need to account for the double adjust here. timeOfLastFileWrite will ALREADY call nowUtc() (which uses the NTP client).
    // Thus, the year will be adjusted already. We shouldn't ever be doing multi year adjustments but it's a possibility to look out for just in case.
    // We need the NON-adjusted year.
    const auto currentLocal = date::make_zoned(
        date::current_zone(),
        shared::time::NTPClient::singleton()->revertTime(shared::filesystem::timeOfLastFileWrite(logPath))
    ).get_local_time();
    std::ostringstream datetime;
    datetime << parts[0] << "/" << date::year_month_day{date::floor<date::days>(currentLocal)}.year() << " " << parts[1];

    const auto t = date::make_zoned(
        date::current_zone(),
        shared::strToLocalTime(datetime.str(), "%m/%d/%Y %T")
    );
    log.timestamp = shared::time::NTPClient::singleton()->adjustTime(t.get_sys_time());

    // Need to combine everything from the 3rd part onwards back
    // into one string since there could be spaces in the tokens
    // that aren't relevant to the initial DATE TIME TOKEN split.
    parts.erase(parts.begin(), parts.begin() + 2);
    const auto tokenPart = boost::join_if(parts, " ", [](const std::string& p) {
        return !boost::trim_copy(p).empty();
    });
    log.rawLog = tokenPart;

    try {
        boost::tokenizer<boost::escaped_list_separator<char>> tok(tokenPart);
        std::copy(tok.begin(), tok.end(), std::back_inserter(log.log));
    } catch (std::exception& ex) {
        // This is generally fine - we'll fail for like "EMOTE" but I mean whatever.
        LOG_WARNING("Failed to tokenize WoW combat log line: " << ex.what() << std::endl << "\t" << line << std::endl);
        return false;
    }
    return true;
}

WoWLogWatcher::WoWLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold):
    BaseLogWatcher(useTimeChecks, timeThreshold) {

}

WoWLogWatcher::~WoWLogWatcher() {
    _running = false;
    if (_timestampLogThread.joinable()) {
        _timestampLogThread.join();
    }
}

void WoWLogWatcher::loadFromExecutable(const fs::path& exePath) {
    const auto combatLogFolder = exePath.parent_path() / fs::path("Logs");
    LOG_INFO("...Timestamped combat log detected." << std::endl);
    // Newer WoW - file will be of the form WoWCombatLog-DATE-TIME.txt.
    _timestampLogThread = std::thread([this, combatLogFolder](){
        std::filesystem::path logFile;
        bool found = false;
        size_t iterations = 0;
        bool isOldFile = true;
        std::optional<typename std::ifstream::pos_type> position = std::nullopt;
        // This loop needs to constantly run until WoW exits to catch each subsequent WoW combat log.
        while (_running) {
            // I'm not sure how to do this conditional, catch the possible exception in timeOfLastFileWrite and *still*
            // continue on in the while loop all at once.
            try {
                if (!fs::exists(logFile) || (shared::filesystem::timeOfLastFileWrite(logFile) < timeThreshold()) || (shared::filesystem::timeOfLastFileWrite(logFile) < _lastLogTime)) {
                    const auto oldLogFile = logFile;
                    logFile = shared::filesystem::getNewestFileInFolder(combatLogFolder, [](const fs::path& path){
                        if (path.extension() != ".txt") {
                            return false;
                        }

                        if (path.filename().native().find(L"WoWCombatLog"s) != 0) {
                            return false;
                        }

                        return true;
                    });

                    if (oldLogFile != logFile) {
                        iterations = 0;
                    }

                    try {
                        if (fs::exists(logFile)) {
                            std::ifstream file(logFile);
                            if (file.is_open()) {
                                file.seekg(0, std::ios_base::end);
                                position = file.tellg();
                            }
                            file.close();
                        }
                    } catch (std::exception& ex) {
                        LOG_WARNING("Failed to get log file position: " << ex.what() << std::endl);
                    }

                    isOldFile = iterations++ > 0;
                } else {
                    const auto lastWriteTime = shared::filesystem::timeOfLastFileWrite(logFile);
                    found = ((lastWriteTime >= timeThreshold()) && (lastWriteTime >= _lastLogTime));
                    if (found && _running) {
                        _lastLogTime = lastWriteTime;
                        found = false;
                        loadFromPath(
                            logFile,
                            true,
                            logFile.filename() == fs::path("WoWCombatLog.txt"),
                            isOldFile,
                            position
                        );
                    }
                }
            } catch (std::exception& ex) {
                LOG_WARNING("Exception when detecting WoW combat log: " << ex.what() << std::endl);
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void WoWLogWatcher::loadFromPath(const std::filesystem::path& combatLogPath, bool loop, bool legacy, bool isOldFile, std::optional<typename std::ifstream::pos_type> position) {
    if (_logPath == combatLogPath) {
        return;
    }

    const auto oldExists = fs::exists(_logPath);
    const auto newExists = fs::exists(combatLogPath);

    _logPath = combatLogPath;
    _logLine = 0;

    using std::placeholders::_1;

    _legacy = legacy || (combatLogPath.filename().native().find(L"WoWCombatLog-"s) == std::string::npos);
    LOG_INFO("WoW Combat Log Path: " << combatLogPath << " " << loop << " [Legacy: " << _legacy << "]" << std::endl
        << "\tOld Exists: " << oldExists << "\tNew Exists: " << newExists << "\tIs Old: " << isOldFile << std::endl);

    // We need to check if an old combat log exists because in that case we don't need to do this check
    // since we'd presumably already have stored the combat log start information.
    const auto needsCombatLogCheck = !oldExists && newExists && isOldFile;
    if (needsCombatLogCheck) {
        LOG_INFO("...Loading a potentially large pre-existing combat log -- searching for combat log start." << std::endl);
        // In the very special scenario where we're loading a combat log that already existed with data, then we want to do a preliminary
        // parse to get the combat log start event since this event isn't necessarily repeated.
        std::ifstream iff(_logPath);
        std::string line;
        int64_t logLine = 0; 
        while (std::getline(iff, line)) {
            RawWoWCombatLog log;
            log.logLine = logLine++;

            if (!parseRawCombatLogLine(line, log, _logPath)) {
                LOG_WARNING("Failed to parse WoW combat log line [prelim]: " << line << std::endl);
                continue;
            }

            WoWCombatLogState state;
            if (parseCombatLogStart(log, state)) {
                LOG_INFO("...Found first instance of combat log start. Using as definitive." << std::endl);
                notify(static_cast<int>(EWoWLogEvents::CombatLogStart), log.timestamp, (void*)&state, false);
                break;
            }
        }
    }

    // Tell any watchers that we've found a new combat log - they can do with that information what they will.
    // But generally, we'll use a single combat log to determine a "session" - probably more reliable than EXE info.
    notify(static_cast<int>(EWoWLogEvents::NewCombatLog), shared::nowUtc(), nullptr, false);

    _watcher = std::make_unique<LogWatcher>(
        combatLogPath,                                                                  // path
        std::bind(&WoWLogWatcher::onCombatLogChange, this, _1),                         // cb
        timeThreshold(),                                                                // timeThreshold
        // We never want to use the log watcher's waitForNewFile feature here because we're doing
        // effectively the same thing in our own thread.
        // 
        // In the legacy case, if the file doesn't already exists then the log watcher will wait for it to
        // exist anwyay. In the non-legacy case, we know that the file is fine since it passed our own time threshold check.
        false,                                                                          // waitForNewFile
        // Legacy vs non-legacy desn't matter here. We should never need to immediately go to the end.
        // If we're reading in a file that already existed and is suddenly being written to then we should
        // be passing in a position to start reading from. If we're reading in a file that just started
        // existing, then the passed in position should not be set or set to 0.
        false,                                                                          // immediatelyGoToEnd
        loop,                                                                           // loop,
        isOldFile ? position : std::nullopt                                             // initialPosition
    );
    _watcher->disableBatching();
}

void WoWLogWatcher::wait() {
    if (_timestampLogThread.joinable()) {
        _timestampLogThread.join();
    }
    _watcher->wait();
}

void WoWLogWatcher::onCombatLogChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        if (ln.empty()) {
            continue;
        }

        RawWoWCombatLog log;
        log.logLine = _logLine++;
        if (!parseRawCombatLogLine(ln, log, _logPath)) {
            LOG_WARNING("Failed to parse WoW combat log line: " << ln << std::endl);
            continue;
        }

        bool parsed = false;

        if (!parsed) {
            WoWCombatLogState state;
            if (parseCombatLogStart(log, state)) {
                notify(static_cast<int>(EWoWLogEvents::CombatLogStart), log.timestamp, (void*)&state, false);
                parsed = true;
            }
        }

        if (!parsed) {
            WoWEncounterStart encounter;
            if (parseEncounterStart(log, encounter)) {
                notify(static_cast<int>(EWoWLogEvents::EncounterStart), log.timestamp, (void*)&encounter);
                parsed = true;
            }
        }

        if (!parsed) {
            WoWChallengeModeStart challenge;
            if (parseChallengeModeStart(log, challenge)) {
                notify(static_cast<int>(EWoWLogEvents::ChallengeModeStart), log.timestamp, (void*)&challenge);
                parsed = true;
            }
        }
 
        if (!parsed) {
            WoWArenaStart arena;
            if (parseArenaStart(log, arena)) {
                notify(static_cast<int>(EWoWLogEvents::ArenaStart), log.timestamp, (void*)&arena);
                parsed = true;
            }
        }

        if (!parsed) {
            WoWZoneChange zone;
            if (parseZoneChange(log, zone)) {
                notify(static_cast<int>(EWoWLogEvents::ZoneChange), log.timestamp, (void*)&zone);
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

        // Every log line needs to get passed to the CombatLogLine event
        notify(static_cast<int>(EWoWLogEvents::CombatLogLine), log.timestamp, (void*)&log, true, true);

        // Check for SPELL_CAST_SUCCESS specifically - we use this to determine who's in the match if the
        // content in question does not spit out COMBATANT_INFO lines.
        if (!parsed) {
            WowSpellCastSuccess cast;
            if (parseSpellCastSuccess(log, cast)) {
                // This is goign to be quite noisy hence we need to manually override and make it quiet.
                notify(static_cast<int>(EWoWLogEvents::SpellCastSuccess), log.timestamp, (void*)&cast, true, true);
                parsed = true;
            }
        }

        // We need to send the "X_END" events after we send the combat log line as the log line only gets 
        // sent if a match is active. Thus if we end the match, no log line will be sent and we want to send
        // the X_END events.
        if (!parsed) {
            WoWEncounterEnd encounter;
            if (parseEncounterEnd(log, encounter)) {
                notify(static_cast<int>(EWoWLogEvents::EncounterEnd), log.timestamp, (void*)&encounter);
                parsed = true;
            }
        }

        if (!parsed) {
            WoWChallengeModeEnd challenge;
            if (parseChallengeModeEnd(log, challenge)) {
                notify(static_cast<int>(EWoWLogEvents::ChallengeModeEnd), log.timestamp, (void*)&challenge);
                parsed = true;
            }
        }

        if (!parsed) {
            WoWArenaEnd arena;
            if (parseArenaEnd(log, arena)) {
                notify(static_cast<int>(EWoWLogEvents::ArenaEnd), log.timestamp, (void*)&arena);
                parsed = true;
            }
        }
    }
}

void WoWLogWatcher::moveLogToBackup() {
    if (!fs::exists(_logPath)) {
        return;
    }

    // Set this to false just in case we were waiting on a log file.
    _running = false;

    // Ensure that we don't mistake a backup file for an actual new combat log.
    if (_timestampLogThread.joinable()) {
        _timestampLogThread.join();
    }

    // Force destruction fo the log watcher to release any handles we might have on this log file.
    _watcher.reset(nullptr);

    fs::rename(_logPath, shared::filesystem::generateTimestampBackupFileName(_logPath));

    // Remove older log files.
    auto dirIter = fs::directory_iterator(_logPath.parent_path());
    std::vector<fs::path> logPaths(fs::begin(dirIter), fs::end(dirIter));

    // Only manage the files that start with WoWCombatLog.
    logPaths.erase(
        std::remove_if(logPaths.begin(), logPaths.end(), [this](const fs::path& pth){
            return pth.filename().native().find(L"WoWCombatLog") == std::filesystem::path::string_type::npos;
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
        {"logLine", logLine },
        {"rawLog", rawLog }
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