#include "api/league_ingame_api.h"
#include "shared/errors/error.h"

#include <nlohmann/json.hpp>

namespace service::api {

LeagueIngameApi::LeagueIngameApi() {
    std::ostringstream host;
    host << "https://127.0.0.1:2999";
    _client = std::make_unique<http::HttpClient>(host.str());
}

LeagueGameStats LeagueIngameApi::getGameStats() {
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

std::string LeagueIngameApi::getActivePlayerName() {
    const std::string path = "/liveclientdata/activeplayername";
    const auto result = _client->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get League In-Game Active Player Name: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

}