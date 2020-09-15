#pragma once

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
    RSOLogin,
};

struct GameLogState {
    bool isInMatch = false;
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

class ValorantLogWatcher {
public:
    ValorantLogWatcher();

    using ValorantLogEventCallback = std::function<void(const shared::TimePoint&, const void*)>;
    void notifyOnEvent(EValorantLogEvents event, const ValorantLogEventCallback& cb) {
        _callbacks[event].push_back(cb);
    }

private:
    void notify(EValorantLogEvents event, const shared::TimePoint& eventTime, const void* data) const;

    void onGameLogChange(const LogLinesDelta& lines);
    void onClientLogChange(const LogLinesDelta& lines);

    std::filesystem::path _gameLogFilename;
    std::filesystem::path _clientLogFilename;

    GameLogState _gameLogState;
    ClientLogState _clientLogState;

    LogWatcherPtr _gameLogWatcher;
    LogWatcherPtr _clientLogWatcher;

    std::unordered_map<EValorantLogEvents, std::vector<ValorantLogEventCallback>> _callbacks;
};

using ValorantLogWatcherPtr = std::unique_ptr<ValorantLogWatcher>;

}