#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "shared/riot/riot.h"
#include "shared/valorant/valorant.h"
#include "shared/time.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace game_event_watcher {

enum class EValorantLogEvents {
    // Public events
    MatchStart,
    MatchEnd,
    RoundBuyStart,
    RoundPlayStart,
    // Events meant for internal use.
    PvpServer,  // Event for when we determine what the PVP API server is.
    RSOLogin,
};

struct GameLogState {
    bool isInMatch = false;
    bool stagedMatchEnd = false;
    shared::valorant::EValorantMap matchMap;
    std::string matchId;
    std::string apiServer;
};

struct ClientLogState {
    std::string rsoToken;
    std::string entitlementToken;
    shared::riot::RiotUser user;

    bool isLoggedIn() const;
};
bool operator==(const ClientLogState& a, const ClientLogState& b);
bool operator!=(const ClientLogState& a, const ClientLogState& b);

class ValorantLogWatcher: public BaseLogWatcher {
public:
    ValorantLogWatcher();

private:
    void onGameLogChange(const LogLinesDelta& lines);
    void onClientLogChange(const LogLinesDelta& lines);

    std::filesystem::path _gameLogFilename;
    std::filesystem::path _clientLogFilename;

    GameLogState _gameLogState;
    ClientLogState _clientLogState;

    LogWatcherPtr _gameLogWatcher;
    LogWatcherPtr _clientLogWatcher;
};

using ValorantLogWatcherPtr = std::unique_ptr<ValorantLogWatcher>;

}