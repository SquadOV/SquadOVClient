#pragma once

#include "http/http_client.h"
#include "game_event_watcher/logs/base_log_watcher.h"

#include <memory>
#include <string>
#include <vector>

namespace service::api {

enum class ELeagueInGameEvent {
    Available,
    Start
};

struct LeagueGameStats {
    std::string gameMode;
    double gameTime;
    std::string mapName;
    int mapNumber;
};

struct LeagueGameEventData {
    int64_t eventId;
    std::string eventName;
    double eventTime;
};

class LeagueIngameApi {
public:
    LeagueIngameApi();

    LeagueGameStats getGameStats() const;
    std::string getActivePlayerName() const;
    std::vector<LeagueGameEventData> getEventData() const;

private:
    std::unique_ptr<http::HttpClient> _client;
};
using LeagueIngameApiPtr = std::shared_ptr<LeagueIngameApi>;

class LeagueIngameApiPoller: public game_event_watcher::BaseLogWatcher {
public:
    LeagueIngameApiPoller(const LeagueIngameApiPtr& api);
    ~LeagueIngameApiPoller();

    void start();
private:
    std::thread _watcher;

    LeagueIngameApiPtr _api;
    bool _running = true;
    int64_t lastEvent = -1;
};
using LeagueIngameApiPollerPtr = std::shared_ptr<LeagueIngameApiPoller>;

}