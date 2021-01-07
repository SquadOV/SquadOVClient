#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "game_event_watcher/hearthstone/hearthstone_log_config.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/log/log.h"

#include <date/tz.h>
#include <regex>

namespace fs = std::filesystem;
using namespace std::string_literals;

namespace game_event_watcher {
namespace {

const std::regex logRegex("(.*):\\s(?:\\[(.*)\\]\\s)?(.*)");
HearthstoneRawLog parseLogLine(const std::string& line) {
    std::smatch matches;
    HearthstoneRawLog log;

    if (!std::regex_search(line, matches, logRegex)) {
        return log;
    }

    if (matches.size() < 3) {
        return log;
    }
    log.canParse = true;
    
    // Hearthstone prints out log times using the local time I believe so we need to convert it into
    // UTC before passing it through the rest of the application.
    const auto t = date::make_zoned(date::current_zone(), shared::strToLocalTime(matches[1].str()));
    log.tm = t.get_sys_time();
    if (matches.size() == 3) {
        log.log = matches[2].str();
    } else if (matches.size() == 4) {
        log.section = matches[2].str();
        log.log = matches[3].str();
    }
    return log;
}

const std::regex filePrintRegex("D\\s(.*?)\\s(.*)");
HearthstoneRawLog parseFilePrintLogLine(const std::string& line, const std::string& section) {
    std::smatch matches;
    HearthstoneRawLog log;

    if (!std::regex_search(line, matches, filePrintRegex)) {
        return log;
    }
    log.canParse = true;

    // This is a little hacky to get a date time from the time-only string
    // in the power log. We assume the time is an ISO valid section of a time
    // string so we just tack on the current day to get the full ISO string that
    // we can parse.
    const auto now = date::make_zoned(date::current_zone(), shared::nowUtc());
    std::ostringstream newTmString;
    newTmString << shared::timeToDateString(now.get_local_time()) << " " << matches[1].str();

    const auto t = date::make_zoned(date::current_zone(), shared::strToLocalTime(newTmString.str()));
    log.tm = t.get_sys_time();
    log.section = section;
    log.log = matches[2].str();
    return log;
}

const std::regex connectRegex("Network.GotoGameServer -- address= (.*):(\\d*), game=(\\d*), client=(\\d*), spectateKey=(.*)\\sreconnecting=(.*)");
bool parseConnectToGameServer(const HearthstoneRawLog& line, HearthstoneGameConnectionInfo& info) {
    std::smatch matches;
    if (!std::regex_search(line.log, matches, connectRegex)) { 
        return false;
    }

    info.ip = matches[1].str();
    info.port = std::stoi(matches[2].str());
    info.gameId = std::stoi(matches[3].str());
    info.clientId = std::stoi(matches[4].str());
    info.spectateKey = matches[5].str();
    info.reconnecting = matches[6].str() == "True";
    return true;
}

bool parseEndingExperiment(const HearthstoneRawLog& line) {
    return line.log.find("Ending Experiment") != std::string::npos;
}

bool parseConnectionError(const HearthstoneRawLog& line) {
    return line.log.find("Error.AddWarning() - header=Error message=The game failed to start because your opponent failed to connect. Please try again.") != std::string::npos;
}

const std::regex startDraftRegex("DraftManager.OnBegin - Got new draft deck with ID: (\\d*)");
bool parseStartArenaDraft(const HearthstoneRawLog& line, int64_t& deckId) {
    std::smatch matches;
    if (!std::regex_search(line.log, matches, startDraftRegex)) { 
        return false;
    }

    deckId = static_cast<int64_t>(std::stoll(matches[1].str()));
    return true;
}

const std::regex continueDraftRegex("DraftManager.OnChoicesAndContents - Draft Deck ID: (\\d*), Hero Card = .*");
bool parseArenaDraftContinue(const HearthstoneRawLog& line, int64_t& deckId) {
    std::smatch matches;
    if (!std::regex_search(line.log, matches, continueDraftRegex)) { 
        return false;
    }

    deckId = static_cast<int64_t>(std::stoll(matches[1].str()));
    return true;
}

const std::regex arenaChooseRegex("Client chooses: .* \\((.*)\\)");
bool parseArenaDraftChoice(const HearthstoneRawLog& line, std::string& cardId) {
    std::smatch matches;
    if (!std::regex_search(line.log, matches, arenaChooseRegex)) { 
        return false;
    }

    cardId = matches[1].str();
    return true;
}

bool parseFinishArenaDraft(const HearthstoneRawLog& line) {
    return line.log.find("SetDraftMode - ACTIVE_DRAFT_DECK") != std::string::npos;
}

}

nlohmann::json HearthstoneGameConnectionInfo::toJson() const {
    return {
        { "ip", ip },
        { "port", port },
        { "gameId", gameId },
        { "clientId", clientId },
        { "spectateKey", spectateKey },
        { "reconnecting", reconnecting}
    };
}

void HearthstoneLogWatcher::enableHearthstoneLogging()
{
    // Deliberate choice to make the Hearthstone local app data folder ahead of time
    // even if user doesn't have Hearthstone installed. This way, we're guaranteed to
    // work as soon as the user launches Hearthstone.
    const auto configPath = shared::filesystem::getHearthstoneAppDataFolder() / fs::path("log.config");
    const auto parentDir = configPath.parent_path();
    LOG_INFO("Hearthstone Log Config file: " << configPath << std::endl);
    if (!fs::exists(parentDir)) {
        LOG_INFO("\tLog File does not exist." << std::endl);
        fs::create_directories(parentDir);
    }

    HearthstoneLogConfig cfg(configPath);
    LOG_INFO("Enable Power" << std::endl);
    cfg.enableLogSection(HearthstoneLogSection::Power);
    LOG_INFO("Enable Arena" << std::endl);
    cfg.enableLogSection(HearthstoneLogSection::Arena);
    LOG_INFO("Enable PVPDR" << std::endl);
    cfg.enableLogSection(HearthstoneLogSection::PvpDr);
    LOG_INFO("Save Config" << std::endl);
    cfg.save();
}

HearthstoneLogWatcher::HearthstoneLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold):
    BaseLogWatcher(useTimeChecks, timeThreshold) {
}

void HearthstoneLogWatcher::loadFromExecutable(const std::filesystem::path& exePath) {
    const auto folder = exePath.parent_path() / fs::path("Logs");
    // The first thing we need to do is to detect the *latest* primary log file.
    // There are multiple log files in this folder so we need to find the one
    // with the latest write time whose name is prefixed with "hearthstone_".
    // This might take awhile to finish as the first file found may not be the found
    // that we want as I think Hearthstone creates the new log after the game starts
    // so we need to kick off a thread to do this work so we don't block.
    std::thread t([this, folder](){
        std::filesystem::path logFile;
        while (true) {
            // I'm not sure how to do this conditional, catch the possible exception in timeOfLastFileWrite and *still*
            // continue on in the while loop all at once.
            try {
                if (!fs::exists(logFile) || (shared::filesystem::timeOfLastFileWrite(logFile) < timeThreshold())) {
                    logFile = shared::filesystem::getNewestFileInFolder(folder, [](const fs::path& path){
                        if (path.extension() != ".log") {
                            return false;
                        }

                        if (path.filename().native().find(L"hearthstone_"s) != 0) {
                            return false;
                        } 

                        return true;
                    });
                } else {
                    break;
                }
            } catch (std::exception& ex) {
                LOG_WARNING("Exception when detecting hearthstone log: " << ex.what() << std::endl);
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        loadPrimaryFromFile(logFile);
    });
    t.detach();

    loadPowerFromFile(folder / fs::path("Power.log"));
    loadArenaFromFile(folder / fs::path("Arena.log"));
}

void HearthstoneLogWatcher::loadPrimaryFromFile(const std::filesystem::path& logFile) {
    LOG_INFO("Hearthstone Primary Log Found: " << logFile << std::endl);
    using std::placeholders::_1;
    _primaryWatcher = std::make_unique<LogWatcher>(logFile, std::bind(&HearthstoneLogWatcher::onPrimaryLogChange, this, _1), timeThreshold(), useTimeChecks());
    _primaryWatcher->disableBatching();
}

void HearthstoneLogWatcher::loadPowerFromFile(const std::filesystem::path& logFile) {
    LOG_INFO("Hearthstone Power Log Found: " << logFile << std::endl);
    using std::placeholders::_1;
    // 'false' -> Don't wait for a new file - we want to capture the Power.log immediately and not just when it's written to because 
    //            if we wait for when the Power.log is written to we'll have missed some crucial information.
    // 'true' -> We want to seek to the end because we want to ignore past matches if they exist in the file.
    _powerWatcher = std::make_unique<LogWatcher>(logFile, std::bind(&HearthstoneLogWatcher::onPowerLogChange, this, _1), timeThreshold(), false, useTimeChecks());
    _powerWatcher->disableBatching();
}

void HearthstoneLogWatcher::loadArenaFromFile(const std::filesystem::path& logFile) {
    LOG_INFO("Hearthstone Arena Log Found: " << logFile << std::endl);
    using std::placeholders::_1;
    _arenaWatcher = std::make_unique<LogWatcher>(logFile, std::bind(&HearthstoneLogWatcher::onArenaLogChange, this, _1), timeThreshold(), false, true);
    _arenaWatcher->disableBatching();
}

void HearthstoneLogWatcher::onArenaLogChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        const auto rawLog = parseFilePrintLogLine(ln, "Arena");
        if (!rawLog.canParse) {
            continue;
        }

        int64_t deckId = 0;
        std::string cardId;
        if (parseStartArenaDraft(rawLog, deckId) || parseArenaDraftContinue(rawLog, deckId)) {
            notify(static_cast<int>(EHearthstoneLogEvents::ArenaStartDraft), rawLog.tm, &deckId);
        } else if (parseFinishArenaDraft(rawLog)) {
            notify(static_cast<int>(EHearthstoneLogEvents::ArenaFinishDraft), rawLog.tm, nullptr);
        } else if (parseArenaDraftChoice(rawLog, cardId)) {
            notify(static_cast<int>(EHearthstoneLogEvents::ArenaDraftChoice), rawLog.tm, &cardId);
        }
    }
}

void HearthstoneLogWatcher::onPowerLogChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        const auto rawLog = parseFilePrintLogLine(ln, "Power");
        if (!rawLog.canParse) {
            continue;
        }

        const auto wasRunning = _powerParser.isGameRunning();
        _powerParser.parse(rawLog);
        const auto isRunning = _powerParser.isGameRunning();

        if (wasRunning != isRunning) {
            LOG_INFO("Detect Running Change: " << wasRunning << " to " << isRunning << std::endl);
            if (isRunning) {
                notify(static_cast<int>(EHearthstoneLogEvents::MatchStart), rawLog.tm, nullptr);
            } else {
                const auto start = shared::nowUtc();
                const auto data = _powerParser.toJson();
                const auto elapsed = shared::nowUtc() - start;

                // It's possible for the power parse toJson function to take awhile - i.e.
                // enough time such that the notification time check fails. The time check is
                // primarily to make sure we don't get an event that's from a previous log file.
                // A way to get around this is to offset the log time to include the processing time.
                // This way if this event *just happened* then we'll still send the notification for this
                // event.
                const auto eventTime = rawLog.tm + elapsed;
                notify(static_cast<int>(EHearthstoneLogEvents::MatchEnd), eventTime, &data);
            }
        }

        _powerParser.postParse();
    }
}

void HearthstoneLogWatcher::onPrimaryLogChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        // First parse the log line into the following format:
        // TIME: [SECTION] INFO
        // Where the log line may or may not have a [SECTION] tag.
        const auto rawLog = parseLogLine(ln);
        if (!rawLog.canParse) {
            continue;
        }

        if (rawLog.section == "HealthyGaming") {
            // Always send the game ready event regardless of whether this is an old line or not
            // since we need to know when to load mono into memory.
            notify(static_cast<int>(EHearthstoneLogEvents::GameReady), rawLog.tm, nullptr, false);
        } else {
            // Parse match connect here as this is the only place this server information will be stored.
            // Note that it is *UNSAFE* to clear the power log here as we're honestly not quite sure on
            // the order of operations between the two threads: the power log thread *could have* picked up
            // the power log already so we don't want to clear out any data that we might need.
            HearthstoneGameConnectionInfo connectionInfo;
            if (parseConnectToGameServer(rawLog, connectionInfo)) {
                notify(static_cast<int>(EHearthstoneLogEvents::MatchConnect), rawLog.tm, &connectionInfo);
            } else if (parseEndingExperiment(rawLog) || parseConnectionError(rawLog)) {
                notify(static_cast<int>(EHearthstoneLogEvents::MatchDisconnect), rawLog.tm, nullptr);
            }
        }
    }
}

void HearthstoneLogWatcher::clearGameState() {
    _powerParser.clear();
}

void HearthstoneLogWatcher::wait() {
    _primaryWatcher->wait();
    _powerWatcher->wait();
}

}