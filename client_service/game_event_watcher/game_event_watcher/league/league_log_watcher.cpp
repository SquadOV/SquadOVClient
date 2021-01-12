#include "game_event_watcher/league/league_log_watcher.h"
#include "shared/filesystem/utility.h"
#include <regex>

namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;

namespace game_event_watcher {

namespace {

const std::string CFG_LINE_TOKEN = "CFG| Command Line:";
const std::regex GAME_ID_REGEX(".*-GameID=(\d*).*");
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

    return false;
}

}


// Don't use time threshold since a new log files gets generated for each match
LeagueLogWatcher::LeagueLogWatcher(const shared::TimePoint& timeThreshold):
    BaseLogWatcher(false, timeThreshold) {

}

void LeagueLogWatcher::loadFromExecutable(const std::filesystem::path& exePath) {
    const auto logFolder = exePath.parent_path() / fs::path("Logs") / fs::path("GameLogs");
    // Need to find the most recent folder and the most recent log file. I'm assuming
    // that the relevant game folder will be setup by the time we get here.
    auto dirIter = fs::directory_iterator(logFolder);
    std::vector<fs::path> gameFolderPaths(fs::begin(dirIter), fs::end(dirIter));

    shared::TimePoint latestTime = shared::zeroTime();
    fs::path latestFolder;

    for (const auto& gameFolder : gameFolderPaths) {
        // Each folder is conveniently named as a UTC time of the format
        // YEAR-MONTH-DATETHOUR-MINUTES-SECONDS. So part that to figure out
        // which folder was made the latest.
        const auto fname = shared::filesystem::pathUtf8(gameFolder.filename());
        const auto tm = shared::strToTime(fname, "%FT%H-%M-%S");
        if (tm > latestTime) {
            latestTime = tm;
            latestFolder = gameFolder;
        }
    }

    if (!fs::exists(latestFolder)) {
        LOG_ERROR("Failed to find League game log: " << exePath << std::endl);
        return;
    }

    // Now we just need to grab the one file that has r3dlog.txt in it.
    dirIter = fs::directory_iterator(latestFolder);
    std::vector<fs::path> logPaths(fs::begin(dirIter), fs::end(dirIter));
    for (const auto& path : logPaths) {
        const auto fname = shared::filesystem::pathUtf8(path.filename());
        if (fname.find("r3dlog.txt") != std::string::npos) {
            loadFromPath(fname);
            break;
        }
    }
}

void LeagueLogWatcher::loadFromPath(const std::filesystem::path& logPath) {
    LOG_INFO("Loading League Logs: " << logPath << std::endl);
    _r3dPath = logPath;
    using std::placeholders::_1;
    _r3dWatcher = std::make_unique<LogWatcher>(_r3dPath, std::bind(&LeagueLogWatcher::onR3dChange, this, _1), timeThreshold(), useTimeChecks(), false, true);
}

void LeagueLogWatcher::onR3dChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        {
            LeagueCommandLineCfg cfg;
            if (parseLeagueCommandLineCfg(ln, cfg)) {
                // We could parse the actual time  but MEH.
                notify(static_cast<int>(ELeagueLogEvents::CommandLineCfg), shared::nowUtc(), &cfg);
            }
        }
    }
}

}