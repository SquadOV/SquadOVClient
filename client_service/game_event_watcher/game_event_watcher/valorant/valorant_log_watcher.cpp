#include "game_event_watcher/valorant/valorant_log_watcher.h"

#include "shared/constants.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <regex>
#include <stdlib.h>
#include <string>

namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;

namespace {

const std::string valorantLogDtFormat = "%Y.%m.%d-%H.%M.%S";

struct ValorantMapChangeData {
    shared::TimePoint logTime;
    std::string map;
    bool ready = false;
    bool complete = false;
};
const std::regex mapChangeRegex("\\[(.*?):\\d{3}\\].*?LogMapLoadModel: Update: \\[Map: (.*)\\] \\[Ready: (.*)\\] \\[Complete: (.*)\\] \\[URL:.*\\]");
bool parseGameLogMapChange(const std::string& line, ValorantMapChangeData& data) {
    std::smatch matches;
    if (!std::regex_search(line, matches, mapChangeRegex)) {
        return false;
    }

    data.logTime = shared::strToTime(matches[1].str(), valorantLogDtFormat);
    data.map = matches[2].str();
    data.ready = matches[3].str() == "TRUE";
    data.complete = matches[4].str() == "TRUE";
    return true;
}

struct ValorantRoundData {
    shared::TimePoint logTime;
    double clientTime;
    double serverTime;
};

const std::regex roundStartRegex("\\[(.*?):\\d{3}\\].*?Gameplay started at local time (.*) \\(server time (.*)\\)");
bool parseRoundChange(const std::string& line, ValorantRoundData& data) {
    std::smatch matches;
    if (!std::regex_search(line, matches, roundStartRegex)) {
        return false;
    }
    data.logTime = shared::strToTime(matches[1].str(), valorantLogDtFormat);
    data.clientTime = std::stod(matches[2].str());
    data.serverTime = std::stod(matches[3].str());
    return true;
}

struct ValorantHTTPQueryData {
    std::string queryName;
    std::string method;
    std::string url;
};
const std::regex httpRegex("LogPlatformCommon: Platform HTTP Query End. QueryName: \\[(.*)\\], URL \\[([A-Z]*)\\s(.*?)\\]");
bool parseGameLogHTTPQuery(const std::string& line, ValorantHTTPQueryData& data) {
    std::smatch matches;
    if (!std::regex_search(line, matches, httpRegex)) {
        return false;
    }

    data.queryName = matches[1].str();
    data.method = matches[2].str();
    data.url = matches[3].str();
    return true;
}

const std::regex loggedInRegex("LogShooterGameUserSettings: Logged in user changed: (.*)");
bool parseLoggedInChange(const std::string& line, std::string& puuid) {
    std::smatch matches;
    if (!std::regex_search(line, matches, loggedInRegex)) {
        return false;
    }

    puuid = matches[1].str();
    return true;
}

std::string getMatchIdFromGameFetchMatch(const std::string& url) {
    // General structure: https://glz-na-1.na.a.pvp.net/core-game/v1/matches/MATCH_ID
    // or: https://glz-na-1.na.a.pvp.net/pregame/v1/matches/a1934cc5-0bb7-4f9e-84c7-2f62f7798feb
    const std::string match = "/v1/matches/";
    const auto matchIdx = url.find(match);
    if (matchIdx == std::string::npos) {
        return "";
    }

    const auto idIdx = matchIdx + match.size();
    return url.substr(idIdx);
}

std::string getApiServer(const std::string& url) {
    // Strip https:// prefix using the offset.
    // Look for the first / to delimit server hostname.
    const auto httpsOffset = 8;
    const auto slashIndex = url.find('/', httpsOffset);
    return url.substr(httpsOffset, slashIndex - httpsOffset);
}

}

namespace game_event_watcher {

ValorantLogWatcher::ValorantLogWatcher() {
    // Find log files which are stored in (Windows)
    //  1) %LOCALAPPDATA%/VALORANT/Saved/Logs (Game Logs)
    //  2) %LOCALAPPDATA%/Riot Games/Riot Client/Logs/Riot Client Logs (Client Logs)
#ifdef _WIN32
    char* localAppData;
    size_t len;
    _dupenv_s(&localAppData, &len, "LOCALAPPDATA");

    const fs::path localAppDataDir(localAppData);
    free(localAppData);

    const fs::path gameLogDir = localAppDataDir / fs::path("VALORANT") / fs::path("Saved") / fs::path("Logs");
#else
    THROW_ERROR("Unsupported OS for Valorant Log Watcher.");
#endif

    if (!fs::exists(gameLogDir)) {
        THROW_ERROR("Failed to find either the game log directory.");
    }

    const fs::path gameLogFname = gameLogDir / fs::path("ShooterGame.log");
    _gameLogFilename = gameLogFname;
    LOG_INFO("VALORANT Game Log: " << _gameLogFilename.string() << std::endl);

    using std::placeholders::_1;
    _gameLogWatcher = std::make_unique<LogWatcher>(_gameLogFilename, std::bind(&ValorantLogWatcher::onGameLogChange, this, _1), true);
}

void ValorantLogWatcher::onGameLogChange(const LogLinesDelta& lines) {
    for (const auto& line : lines) {
        const GameLogState previousState = _gameLogState;
        bool parsed = false;
        {
            ValorantMapChangeData data;
            if (parseGameLogMapChange(line, data) && data.ready) {
                _gameLogState.matchMap = shared::valorant::codenameToValorantMap(data.map);
                _gameLogState.isInMatch = (previousState.matchId != "" && shared::valorant::isGameMap(_gameLogState.matchMap));

                if (_gameLogState.isInMatch && _gameLogState.matchMap == previousState.matchMap && data.complete) {
                    // This indicates that we're loading out of a match at this point.
                    // We're going to assume that this means the game is finished. We're going to
                    // mark the game as truly finished once they get back to the main menu.
                    _gameLogState.stagedMatchEnd = true;
                } else if (_gameLogState.matchMap == shared::valorant::EValorantMap::MainMenu &&
                    (previousState.isInMatch || previousState.matchMap == shared::valorant::EValorantMap::Init) &&
                    !_gameLogState.matchId.empty()) {
                    // We've loaded back into the main menu - fire off the match end event.
                    // There's two possibilites here: 
                    //  1) the user just straight left the game
                    //  2) the game actually ended.
                    // We should be able to differentiate the two using the "stagedMatchEnd" variable. Leave it
                    // up to the callback to differentiate.
                    notify(static_cast<int>(EValorantLogEvents::MatchEnd), data.logTime, &_gameLogState);

                    // We should be able to clear out the state at this point since the client
                    // should have processed the state data and since the game ended we know we can
                    // start anew with the next game. Clear out everything besides the apiServer since
                    // that should stay the same.
                    _gameLogState.isInMatch = false;
                    _gameLogState.matchId = "";
                    _gameLogState.matchMap = shared::valorant::EValorantMap::Unknown;
                    _gameLogState.stagedMatchEnd = false;
                } else if (_gameLogState.isInMatch) {
                    notify(static_cast<int>(EValorantLogEvents::MatchStart), data.logTime, &_gameLogState);
                }
                
                parsed = true;
            }
        }

        if (!parsed) {
            ValorantRoundData data;
            if (parseRoundChange(line, data)) {
                // Need to use server time and not client time here because in a real game
                // the client tiem might not start at 0.0.
                if (data.serverTime < shared::EPSILON) {
                    notify(static_cast<int>(EValorantLogEvents::RoundBuyStart), data.logTime, nullptr);
                } else {
                    notify(static_cast<int>(EValorantLogEvents::RoundPlayStart), data.logTime, nullptr);
                }
                parsed = true;
            }
        }

        if (!parsed) {
            ValorantHTTPQueryData data;
            if (parseGameLogHTTPQuery(line, data)) {
                if (data.queryName == "CoreGame_FetchMatch" || data.queryName == "Pregame_GetMatch") {
                    _gameLogState.matchId = getMatchIdFromGameFetchMatch(data.url);
                    parsed = true;
                } else if (data.queryName == "DisplayNameService_UpdatePlayer") {
                    // Use this query to determine the API server to hit since it'll be
                    // fired near the beginning of the log using the same API server we can use
                    // to get the match history and such.
                    _gameLogState.apiServer = getApiServer(data.url);
                    notify(static_cast<int>(EValorantLogEvents::PvpServer), shared::nowUtc(), &_gameLogState.apiServer);
                    parsed = true;
                }
            }
        }

        if (!parsed) {
            if (parseLoggedInChange(line, _gameLogState.puuid)) {
                notify(static_cast<int>(EValorantLogEvents::RSOLogin), shared::nowUtc(), &_gameLogState.puuid);
                parsed = true;
            }
        }
    }
}
    
}