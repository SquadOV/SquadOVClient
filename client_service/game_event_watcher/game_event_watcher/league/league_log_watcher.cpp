#include "game_event_watcher/league/league_log_watcher.h"
#include "shared/filesystem/utility.h"
#include "shared/time/ntp_client.h"
#include <regex>

namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;

namespace game_event_watcher {

namespace {

const std::string CFG_LINE_TOKEN = "CFG| Command Line:";
const std::regex GAME_ID_REGEX(".*-GameID=(\\d*).*");
const std::regex REGION_REGEX(".*-Region=(.*?)\".*");
const std::regex PLATFORM_REGEX(".*-PlatformID=(.*?)\".*");
bool parseLeagueCommandLineCfg(const std::string& line, LeagueCommandLineCfg& cfg) {
    if (line.find(CFG_LINE_TOKEN) == std::string::npos) {
        return false;
    }

    {
        std::smatch matches;
        if (!std::regex_search(line, matches, GAME_ID_REGEX)) {
            return false;
        }
        cfg.gameId = static_cast<int64_t>(std::stoll(matches[1].str()));
    }

    {
        std::smatch matches;
        if (!std::regex_search(line, matches, REGION_REGEX)) {
            return false;
        }
        cfg.region = matches[1].str();
    }

    {
        std::smatch matches;
        if (!std::regex_search(line, matches, PLATFORM_REGEX)) {
            return false;
        }
        cfg.platformId = matches[1].str();
    }

    return true;
}

const std::regex CONNECTION_READY_REGEX("CONNECTION READY \\| (?:TeamOrder|TeamChaos) \\d*\\)\\s(.*)\\s\\*\\*LOCAL\\*\\*\\s-.*PUUID\\((.*?)\\)");
bool parseLeagueLocalPlayer(const std::string& line, LeagueLocalPlayer& player) {
    std::smatch matches;
    if (!std::regex_search(line, matches, CONNECTION_READY_REGEX)) {
        return false;
    }

    player.name = matches[1].str();
    player.puuid = matches[2].str();
    return true;
}

}


// Don't use time threshold since a new log files gets generated for each match
LeagueLogWatcher::LeagueLogWatcher(const shared::TimePoint& timeThreshold):
    BaseLogWatcher(false, timeThreshold) {

}

void LeagueLogWatcher::loadFromExecutable(const std::filesystem::path& exePath) {
    const auto logFolder = exePath.parent_path().parent_path() / fs::path("Logs") / fs::path("GameLogs");
    
    // Need to use the current time with some negative buffer for inexact timings to find the 
    // the right log folder to use. Note that this directory may not be immediately available
    // hence why this needs to be started up in a separate thread.
    std::thread t([this, logFolder]() {
        const shared::TimePoint thresholdTime = shared::nowUtc() - std::chrono::minutes(5);
        bool found = false;
        while (!found) {
            // Need to find the most recent folder and the most recent log file. I'm assuming
            // that the relevant game folder will be setup by the time we get here.
            auto dirIter = fs::directory_iterator(logFolder);

            auto latestTime = thresholdTime;
            std::vector<fs::path> gameFolderPaths(fs::begin(dirIter), fs::end(dirIter));
            fs::path latestFolder;

            for (const auto& gameFolder : gameFolderPaths) {
                // Each folder is conveniently named as a local time in the format
                // YEAR-MONTH-DATETHOUR-MINUTES-SECONDS. So parse that to figure out
                // which folder was made the latest.
                const auto fname = shared::filesystem::pathUtf8(gameFolder.filename());
                const auto tm = shared::time::NTPClient::singleton()->adjustTime(date::make_zoned(
                    date::current_zone(),
                    shared::strToLocalTime(fname, "%FT%H-%M-%S")
                ).get_sys_time());
                if (tm > latestTime) {
                    latestTime = tm;
                    latestFolder = gameFolder;
                }
            }

            if (!fs::exists(latestFolder)) {
                LOG_WARNING("Failed to find League game log...retrying: " << logFolder << std::endl);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                continue;
            }

            // Now we just need to grab the one file that has r3dlog.txt in it.
            dirIter = fs::directory_iterator(latestFolder);
            std::vector<fs::path> logPaths(fs::begin(dirIter), fs::end(dirIter));
            for (const auto& path : logPaths) {
                const auto fname = shared::filesystem::pathUtf8(path.filename());
                if (fname.find("r3dlog.txt") != std::string::npos) {
                    loadFromPath(path);
                    found = true;
                    break;
                }
            }

            if (!found) {
                LOG_WARNING("Failed to find r3dlog file in dir: " << latestFolder << std::endl);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    });
    t.detach();
    
}

void LeagueLogWatcher::loadFromPath(const std::filesystem::path& logPath) {
    LOG_INFO("Loading League Logs: " << logPath << std::endl);
    _r3dPath = logPath;
    using std::placeholders::_1;
    _r3dWatcher = std::make_unique<LogWatcher>(_r3dPath, std::bind(&LeagueLogWatcher::onR3dChange, this, _1), timeThreshold(), useTimeChecks(), false, true);
}

void LeagueLogWatcher::onR3dChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        bool parsed = false;
        {
            LeagueCommandLineCfg cfg;
            if (parseLeagueCommandLineCfg(ln, cfg)) {
                // We could parse the actual time  but MEH.
                notify(static_cast<int>(ELeagueLogEvents::CommandLineCfg), shared::nowUtc(), &cfg);
                parsed = true;
            }
        }

        if (!parsed) {
            LeagueLocalPlayer player;
            if (parseLeagueLocalPlayer(ln, player)) {
                notify(static_cast<int>(ELeagueLogEvents::LocalPlayer), shared::nowUtc(), &player);
                parsed = true;
            }
        }
    }
}

void LeagueLogWatcher::wait() {
    _r3dWatcher->wait();
}

}