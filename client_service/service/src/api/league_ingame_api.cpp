#include "api/league_ingame_api.h"
#include "shared/errors/error.h"
#include "shared/time.h"

#include <nlohmann/json.hpp>

namespace service::api {

LeagueIngameApi::LeagueIngameApi() {
    std::ostringstream host;
    host << "https://127.0.0.1:2999";
    _client = std::make_unique<http::HttpClient>(host.str());
    _client->enableSelfSigned();
    _client->quiet();
}

LeagueGameStats LeagueIngameApi::getGameStats() const {
    const std::string path = "/liveclientdata/gamestats";
    const auto result = _client->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get League In-Game Game Stats: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    LeagueGameStats ret;
    ret.gameMode = parsedJson["gameMode"].get<std::string>();
    ret.gameTime = parsedJson["gameTime"].get<double>();
    ret.mapName = parsedJson["mapName"].get<std::string>();
    ret.mapNumber = parsedJson["mapNumber"].get<int>();
    return ret;
}

std::string LeagueIngameApi::getActivePlayerName() const {
    const std::string path = "/liveclientdata/activeplayername";
    const auto result = _client->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get League In-Game Active Player Name: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::vector<LeagueGameEventData> LeagueIngameApi::getEventData() const {
    const std::string path = "/liveclientdata/eventdata";
    const auto result = _client->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get League In-Game Events: " << result->status);
        return {};
    }

    std::vector<LeagueGameEventData> ret;
    const auto parsedJson = nlohmann::json::parse(result->body);
    for (const auto& ev: parsedJson["Events"]) {
        LeagueGameEventData data;
        data.eventId = ev["EventID"].get<int64_t>();
        data.eventName = ev["EventName"].get<std::string>();
        data.eventTime = ev["EventTime"].get<double>();
        ret.push_back(data);
    }
    return ret;
}

LeagueIngameApiPoller::LeagueIngameApiPoller(const LeagueIngameApiPtr& api):
    game_event_watcher::BaseLogWatcher(false, shared::nowUtc()),
    _api(api) {
}

void LeagueIngameApiPoller::start() {
    _watcher = std::thread([this](){
        bool firstAvailable = false;
        bool available = false;
        while (_running) {
            // Ping the game stats endpoint since we're going to need that data anyway.
            LeagueGameStats stats;
            const auto tm = shared::nowUtc();
            try {
                stats = _api->getGameStats();
                available = true;
            } catch(std::exception& ex) {
                LOG_WARNING("Failed to ping LoL Client API - assuming it's not yet available: " << ex.what() << std::endl);
            }

            if (available) {
                if (!firstAvailable) {
                    notify(static_cast<int>(ELeagueInGameEvent::Available), shared::nowUtc(), reinterpret_cast<const void*>(&stats), false);
                    firstAvailable = true;
                }

                try {
                    const auto events = _api->getEventData();
                    if (!events.empty()) {
                        for (const auto& event: events) {
                            if (event.eventId <= lastEvent) {
                                continue;
                            }

                            // Note that we might not have picked the *exact* moment the event came up.
                            // Thus to compute the actual time, we need to offset from the time we sent the
                            // getGameStats request by how much the event time differs from the game time.
                            const auto eventTime = tm - std::chrono::milliseconds(static_cast<long long>((stats.gameTime - event.eventTime) * 1000.0));
                            if (event.eventName == "GameStart") {
                                notify(static_cast<int>(ELeagueInGameEvent::Start), eventTime, nullptr, false);
                            }

                            lastEvent = event.eventId;
                        }
                    }
                } catch (std::exception& ex) {
                    LOG_WARNING("Failed to get event data: " << ex.what() << std::endl);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    });
}

LeagueIngameApiPoller::~LeagueIngameApiPoller() {
    _running = false;
    if (_watcher.joinable()) {
        _watcher.join();
    }
}

}