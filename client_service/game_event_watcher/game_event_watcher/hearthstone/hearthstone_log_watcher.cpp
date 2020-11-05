#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "game_event_watcher/hearthstone/hearthstone_log_config.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/log/log.h"

#include <date/tz.h>
#include <regex>

namespace fs = std::filesystem;

namespace game_event_watcher {
namespace {

const std::regex logRegex("(.*):\\s(?:\\[(.*)\\]\\s)?(.*)");
HearthstoneRawLog parseLogLine(const std::string& line) {
    std::smatch matches;
    HearthstoneRawLog log;

    if (!std::regex_search(line, matches, logRegex)) {
        LOG_WARNING("Unknown Hearthstone log line: " << line << std::endl);
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

bool parseExperimentEnd(const HearthstoneRawLog& line) {
    return line.log.find("Ending Experiment") == 0;
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
    if (!fs::exists(configPath)) {
        fs::create_directories(configPath.parent_path());
    }

    HearthstoneLogConfig cfg(configPath);
    cfg.enableLogSection(HearthstoneLogSection::Power);
    cfg.save();
}

HearthstoneLogWatcher::HearthstoneLogWatcher(bool useTimeChecks):
    BaseLogWatcher(useTimeChecks) {
}

void HearthstoneLogWatcher::loadFromExecutable(const std::filesystem::path& exePath) {
    const auto folder = exePath.parent_path() / fs::path("Logs");
    // The first thing we need to do is to detect the *latest* log file.
    // There are multiple log files in this folder so we need to find the one
    // with the latest write time whose name is prefixed with "hearthstone_".
    // This might take awhile to finish as the first file found may not be the found
    // that we want as I think Hearthstone creates the new log after the game starts
    // so we need to kick off a thread to do this work so we don't block.
    std::thread t([this, folder](){
        std::filesystem::path logFile;
        while (!fs::exists(logFile) || (shared::filesystem::secondsSinceLastFileWrite(logFile) > std::chrono::seconds(30))) {
            logFile = shared::filesystem::getNewestFileInFolder(folder, [](const fs::path& path){
                if (path.extension() != ".log") {
                    return false;
                }

                if (path.filename().string().find("hearthstone_") != 0) {
                    return false;
                } 

                return true;
            });
        }
        loadFromFile(logFile);
    });
    t.detach();
}

void HearthstoneLogWatcher::loadFromFile(const std::filesystem::path& logFile) {
    LOG_INFO("Hearthstone Log Found: " << logFile.string() << std::endl);
    using std::placeholders::_1;
    _watcher = std::make_unique<LogWatcher>(logFile, std::bind(&HearthstoneLogWatcher::onLogChange, this, _1), useTimeChecks());
}

void HearthstoneLogWatcher::onLogChange(const LogLinesDelta& lines) {
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
        } else if (rawLog.section == "Power") {
            _powerParser.parse(rawLog);
        } else {
            // There's mainly two events that we want to detect here - game start and game end.
            // Game Start: TIME: Network.GotoGameServer -- address= IP:PORT, game=GAMEID, client=CLIENTID, spectateKey=SPECTATEKEY reconnecting=RECON
            // Game End: Ending Experiment
            //   Note the "Game End" event is a little inconsistent as it might signal something else (I think it's more of a generic scence change ending log line)
            //   but it it logged when the game ends so I think it's safe - the user will just have to track some state.
            HearthstoneGameConnectionInfo connectionInfo;
            if (parseConnectToGameServer(rawLog, connectionInfo)) {
                notify(static_cast<int>(EHearthstoneLogEvents::MatchStart), rawLog.tm, &connectionInfo);
            } else if (parseExperimentEnd(rawLog)) {
                nlohmann::json data = _powerParser.toJson();
                notify(static_cast<int>(EHearthstoneLogEvents::MatchEnd), rawLog.tm, &data);
            }
        }
    }
}

void HearthstoneLogWatcher::clearGameState() {
    _powerParser.clear();
}

void HearthstoneLogWatcher::wait() {
    _watcher->wait();
}

}