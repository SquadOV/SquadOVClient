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
    std::string username;
    std::string tagline;
    std::string puuid;
};

class ValorantLogWatcher: public BaseLogWatcher {
public:
    ValorantLogWatcher();

private:
    void onGameLogChange(const LogLinesDelta& lines);

    std::filesystem::path _gameLogFilename;
    GameLogState _gameLogState;
    LogWatcherPtr _gameLogWatcher;
};

using ValorantLogWatcherPtr = std::unique_ptr<ValorantLogWatcher>;

}