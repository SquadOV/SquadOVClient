#include "game_event_watcher/valorant/valorant_log_watcher.h"

#include "shared/constants.h"


#include <chrono>
#include <filesystem>
#include <iostream>
#include <regex>
#include <stdlib.h>
#include <string>
#include <tinyxml2.h>

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
    double time;
};
const std::regex roundStartRegex("\\[(.*?):\\d{3}\\].*?Gameplay started at local time (.*) \\(server time .*\\)");
bool parseRoundChange(const std::string& line, ValorantRoundData& data) {
    std::smatch matches;
    if (!std::regex_search(line, matches, roundStartRegex)) {
        return false;
    }
    data.logTime = shared::strToTime(matches[1].str(), valorantLogDtFormat);
    data.time = std::stod(matches[2].str());
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

struct ValorantXmppData {
    tinyxml2::XMLDocument packet;
};
const std::regex xmppRegex("chat: XmppTcpSocket.*xmpp: (.*)");
bool parseClientLogXmpp(const std::string& line, ValorantXmppData& data) {
    std::smatch matches;
    if (!std::regex_search(line, matches, xmppRegex)) {
        return false;
    }

    data.packet.Parse(matches[1].str().c_str());
    return true;
}

bool parseXmppRSOToken(const tinyxml2::XMLDocument& doc, std::string& token) {
    // Look for the RSO token case.
    // <auth>
    //    <rso_token>...</rso_token>
    // </auth>
    const auto authNode = doc.FirstChildElement("auth");
    if (!authNode) {
        return false;
    }

    const auto rsoTokenNode = authNode->FirstChildElement("rso_token");
    if (!rsoTokenNode) {
        return false;
    }

    token = rsoTokenNode->GetText();
    return true;
}

bool parseXmppEntitlementToken(const tinyxml2::XMLDocument& doc, std::string& token) {
    // Look for the entitlements case.
    // <iq type="set" id="">
    //    <entitlements>
    //        <token>...</token>
    //    </entitlements>
    // </iq>
    const auto iqNode = doc.FirstChildElement("iq");
    if (!iqNode) {
        return false;
    }

    const std::string iqType = iqNode->Attribute("type");
    const std::string iqId = iqNode->Attribute("id");

    if (iqType != "set") {
        return false;
    }

    const auto entitleNode = iqNode->FirstChildElement("entitlements");
    if (!entitleNode) {
        return false;
    }

    const auto tokenNode = entitleNode->FirstChildElement("token");
    if (!tokenNode) {
        return false;
    }

    token = tokenNode->GetText();
    return true;
}

bool parseXmppPuuid(const tinyxml2::XMLDocument& doc, std::string& puuid) {
    // Look for the puuid case.
    // <iq id='_xmpp_bind1' type='result'>
    //     <bind>
    //         <jid>...@</jid>
    //     </bind>
    // </iq>
    const auto iqNode = doc.FirstChildElement("iq");
    if (!iqNode) {
        return false;
    }

    const std::string iqType = iqNode->Attribute("type");
    const std::string iqId = iqNode->Attribute("id");

    if (iqType != "result") {
        return false;
    }

    const auto bindNode = iqNode->FirstChildElement("bind");
    if (!bindNode) {
        return false;
    }

    const auto jidNode = bindNode->FirstChildElement("jid");
    if (!jidNode) {
        return false;
    }

    const std::string jid = jidNode->GetText();
    puuid = jid.substr(0, jid.find("@"));
    return true;
}

bool parseXmppUsernameTag(const tinyxml2::XMLDocument& doc, std::string& username, std::string& tag) {    
    // Look for the username/tag case.
    // <iq id='_xmpp_session1' type='result'>
    //     <session>
    //         <id name='...' tagline='...' />
    //     </session>
    // </iq>
    const auto iqNode = doc.FirstChildElement("iq");
    if (!iqNode) {
        return false;
    }

    const std::string iqType = iqNode->Attribute("type");
    const std::string iqId = iqNode->Attribute("id");

    if (iqType != "result") {
        return false;
    }

    const auto sessionNode = iqNode->FirstChildElement("session");
    if (!sessionNode) {
        return false;
    }

    const auto idNode  = sessionNode->FirstChildElement("id");
    if (!idNode) {
        return false;
    }

    username = idNode->Attribute("name");
    tag = idNode->Attribute("tagline");
    return true;
}

}

namespace game_event_watcher {

bool operator==(const ClientLogState& a, const ClientLogState& b) {
    return a.entitlementToken == b.entitlementToken &&
        a.rsoToken == b.rsoToken &&
        a.user == b.user;
}
bool operator!=(const ClientLogState& a, const ClientLogState& b) {
    return !(a == b);
}

bool ClientLogState::isLoggedIn() const {
    return !rsoToken.empty() &&
        !entitlementToken.empty() &&
        !user.puuid.empty() &&
        !user.username.empty() &&
        !user.tag.empty();
}

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
    const fs::path clientLogDir = localAppDataDir / fs::path("Riot Games") / fs::path("Riot Client") / fs::path("Logs") / fs::path("Riot Client Logs");
#else
    throw std::runtime_error("Unsupported OS for Valorant Log Watcher.");
#endif

    if (!fs::exists(gameLogDir) || !fs::exists(clientLogDir)) {
        throw std::runtime_error("Failed to find either the game log directory or client log directory.");
    }

    const fs::path gameLogFname = gameLogDir / fs::path("ShooterGame.log");
    _gameLogFilename = gameLogFname;

    auto latestWriteTime = fs::file_time_type::min();
    for (const auto& entry : fs::recursive_directory_iterator(clientLogDir)) {
        const auto path = entry.path();
        if (path.extension() != ".log") {
            continue;
        }

        const auto lastWriteTime = fs::last_write_time(path);
        if (lastWriteTime > latestWriteTime) {
            latestWriteTime = lastWriteTime;
            _clientLogFilename = path;
        }
    }

    std::cout << "VALORANT Game Log: " << _gameLogFilename.string() << std::endl;
    std::cout << "VALORANT Client Log: " << _clientLogFilename.string() << std::endl;

    using std::placeholders::_1;
    _gameLogWatcher = std::make_unique<LogWatcher>(_gameLogFilename, std::bind(&ValorantLogWatcher::onGameLogChange, this, _1), true);
    _clientLogWatcher = std::make_unique<LogWatcher>(_clientLogFilename, std::bind(&ValorantLogWatcher::onClientLogChange, this, _1));
}

void ValorantLogWatcher::onGameLogChange(const LogLinesDelta& lines) {
    for (const auto& line : lines) {
        const GameLogState previousState = _gameLogState;
        bool parsed = false;
        {
            ValorantMapChangeData data;
            if (parseGameLogMapChange(line, data) && data.ready) {        
                _gameLogState.matchMap = shared::valorant::codenameToValorantMap(data.map);
                _gameLogState.isInMatch = (data.ready && !data.complete && previousState.matchId != "" && shared::valorant::isGameMap(_gameLogState.matchMap));
                parsed = true;
                
                if (_gameLogState.isInMatch != previousState.isInMatch) {
                    if (_gameLogState.isInMatch) {
                        notify(EValorantLogEvents::MatchStart, data.logTime, &_gameLogState);
                    } else {
                        notify(EValorantLogEvents::MatchEnd, data.logTime, &_gameLogState);

                        // We should be able to clear out the state at this point since the client
                        // should have processed the state data and since the game ended we know we can
                        // start anew with the next game.
                        _gameLogState = GameLogState{};
                    }
                }
            }
        }

        if (!parsed) {
            ValorantRoundData data;
            if (parseRoundChange(line, data)) {
                if (data.time < shared::EPSILON) {
                    notify(EValorantLogEvents::RoundBuyStart, data.logTime, nullptr);
                } else {
                    notify(EValorantLogEvents::RoundPlayStart, data.logTime, nullptr);
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
                    notify(EValorantLogEvents::PvpServer, shared::nowUtc(), &_gameLogState.apiServer);
                    parsed = true;
                }
            }
        }
    }
}

void ValorantLogWatcher::onClientLogChange(const LogLinesDelta& lines) {
    for (const auto& line : lines) {
        bool parsed = false;
        const ClientLogState previousState = _clientLogState;

        {
            ValorantXmppData data;
            if (parseClientLogXmpp(line, data)) {

                if (parseXmppRSOToken(data.packet, _clientLogState.rsoToken)) {
                    goto success_xmpp_parse;
                }
                
                if (parseXmppEntitlementToken(data.packet, _clientLogState.entitlementToken)) {
                    goto success_xmpp_parse;
                }

                if (parseXmppPuuid(data.packet, _clientLogState.user.puuid)) {
                    goto success_xmpp_parse;
                }

                if (parseXmppUsernameTag(data.packet, _clientLogState.user.username, _clientLogState.user.tag)) {
                    goto success_xmpp_parse;
                }

                goto finish_xmpp_parse;
success_xmpp_parse:
                parsed = true;
finish_xmpp_parse:
                ;
            }
        }

        if (!parsed) {
            continue;
        }

        // This needs to be called repeatedly to detect changes to the entitlement token.
        if (_clientLogState != previousState) {
            if (_clientLogState.isLoggedIn()) {
                // The time here doesn't really matter so just use the current time whatever it is and not the log time.
                notify(EValorantLogEvents::RSOLogin, shared::nowUtc(), &_clientLogState);
            }
        }
    }
}

void ValorantLogWatcher::notify(EValorantLogEvents event, const shared::TimePoint& eventTime, const void* data) const {
    for (const auto& cb : _callbacks.at(event)) {
        cb(eventTime, data);
    }
}
    
}