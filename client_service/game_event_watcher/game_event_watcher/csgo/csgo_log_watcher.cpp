#include "game_event_watcher/csgo/csgo_log_watcher.h"
#include "shared/filesystem/common_paths.h"
#include "shared/errors/error.h"
#include "shared/time/ntp_client.h"

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <optional>
#include <string>
#include <regex>

namespace fs = std::filesystem;
namespace game_event_watcher {
namespace {

struct CsgoLogLine {
    shared::TimePoint tm;
    std::string rest;
    bool parsed = false;
};

const std::regex logRegex("(.*?):\\s(.*)");
CsgoLogLine parseCsgoLog(const std::string& line) {
    std::smatch matches;
    CsgoLogLine log;

    if (!std::regex_search(line, matches, logRegex) || matches.size() < 3) {
        return log;
    }

    log.parsed = true;
    log.tm = shared::time::NTPClient::singleton()->adjustTime(date::make_zoned(date::current_zone(), shared::strToLocalTime(matches[1].str(), "%m/%d/%Y - %H:%M:%S")).get_sys_time());
    log.rest = matches[2].str();

    return log;
}

const std::string serverConnectToken = "Connected to ";
std::optional<std::string> parseServerConnect(const std::string& line) {
    if (!boost::starts_with(line, serverConnectToken)) {
        return std::nullopt;
    }

    return line.substr(serverConnectToken.size());
}

const std::string serverDisconnectToken = "ChangeGameUIState: CSGO_GAME_UI_STATE_INGAME -> CSGO_GAME_UI_STATE_MAINMENU";
bool parseServerDisconnect(const std::string& line) {
    return line.find(serverDisconnectToken) != std::string::npos;
}

}

CsgoLogWatcher::~CsgoLogWatcher() {
    // We know for a fact that we want to delete the log file because CSGO will reuse that file over and over.
    if (_gameLogWatcher && fs::exists(_gameLogWatcher->path())) {
        const auto logPath = _gameLogWatcher->path();
        _gameLogWatcher.reset(nullptr);
        fs::remove(logPath);
    }
}

void CsgoLogWatcher::enableCsgoLogging() {
    const auto csgoPath = shared::filesystem::getCsgoInstallFolder();
    if (!csgoPath.has_value()) {
        LOG_WARNING("Could not find CS:GO installation directory...skipping log initialization." << std::endl);
        return;
    }

    const auto cfgDirectory = shared::filesystem::getCsgoCfgFolder();
    if (!cfgDirectory.has_value() || !fs::exists(cfgDirectory.value())) {
        LOG_WARNING("Could not find CSGO cfg directory for logging.");
        return;
    }

    const auto autoExecCfg = shared::filesystem::getCsgoAutoExecFile();
    if (!autoExecCfg.has_value()) {
        LOG_WARNING("Failed to determine proper autoexec path for CSGO." << std::endl);
        return;
    }

    LOG_INFO("Enabling CSGO Logging with Autoexec File: " << autoExecCfg.value() << std::endl);

    std::vector<std::string> cfgLines;
    if (fs::exists(autoExecCfg.value())) {
        std::ifstream f(autoExecCfg.value());
        std::string line;
        while (f.is_open() && std::getline(f, line)) {
            // So the two autoexec files we're concerned with are "con_logfile" and "con_timestamp".
            // "con_timestamp" is necessary to get timestamped logs while "con_logfile" is necessary
            // to get an actual log file that we can watch.
            if ((line.find("con_logfile") != std::string::npos) || (line.find("con_timestamp") != std::string::npos)) {
                continue;
            }
            cfgLines.push_back(line);
        }
        f.close();
    }

    cfgLines.push_back("con_logfile squadov.log");
    cfgLines.push_back("con_timestamp 1");

    std::ofstream f(autoExecCfg.value());
    for (const auto& l : cfgLines) {
        f << l << std::endl;
    }
    f.close();
}

CsgoLogWatcher::CsgoLogWatcher(const shared::TimePoint& timeThreshold):
    BaseLogWatcher(true, timeThreshold) {

    const auto logPath = shared::filesystem::getCsgoLogFile();
    if (!logPath.has_value()) {
        THROW_ERROR("CS:GO Folder Setup is not as expected so we can't start CS:GO log watcher.");
    }
    LOG_INFO("CS:GO Game Log: " << logPath.value() << std::endl);

    using std::placeholders::_1;
    _gameLogWatcher = std::make_unique<LogWatcher>(logPath.value(), std::bind(&CsgoLogWatcher::onGameLogChange, this, _1), timeThreshold, true);
}

void CsgoLogWatcher::onGameLogChange(const LogLinesDelta& lines) {
    for (const auto& l : lines) {
        const auto log = parseCsgoLog(l);
        if (!log.parsed) {
            continue;
        }

        {
            const auto server = parseServerConnect(log.rest);
            if (server) {
                notify(static_cast<int>(ECsgoLogEvents::Connect), log.tm, (void*)&server.value());
                continue;
            }
        }

        if (parseServerDisconnect(log.rest)) {
            notify(static_cast<int>(ECsgoLogEvents::Disconnect), log.tm, nullptr);
            continue;
        }
    }
}

}