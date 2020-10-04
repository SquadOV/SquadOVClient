#include "api/squadov_api.h"

#include "shared/env.h"
#include "shared/errors/error.h"

#include <nlohmann/json.hpp>
#include <sstream>

namespace service::api {

SquadovApi::SquadovApi() {
    // TODO: Need some better way of communicating this OR just removing it altogether and not
    // having this reliance.
    std::ostringstream host;
    host << "https://127.0.0.1:" << shared::getEnv("SQUADOV_API_PORT", "50000");
    _client = std::make_unique<http::HttpClient>(host.str());
    _client->setBearerAuthToken(shared::getEnv("SQUADOV_API_KEY", "AAAAA"));
    _client->enableSelfSigned();
}

shared::riot::RiotRsoToken SquadovApi::refreshValorantRsoToken(const std::string& puuid) const {
    shared::riot::RiotRsoToken token;
    token.puuid = puuid;

    std::ostringstream path;
    path << "/valorant/accounts/" << puuid << "/rso";

    const auto result = _client->Get(path.str().c_str());

    if (result->status != 200) {
        THROW_ERROR("Failed to obtain RSO token: " << result->status);
        return token;
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    token.rsoToken = parsedJson["token"].get<std::string>();
    token.entitlementsToken = parsedJson["entitlements"].get<std::string>();
    token.expires = shared::strToTime(parsedJson["expires"].get<std::string>(), "%FT%T");
    return token;
}

shared::riot::RiotUser SquadovApi::getRiotUserFromPuuid(const std::string& puuid) const {
    shared::riot::RiotUser user;
    user.puuid = puuid;

    std::ostringstream path;
    path << "/valorant/accounts/" << puuid;

    const auto result = _client->Get(path.str().c_str());

    if (result->status != 200) {
        THROW_ERROR("Failed to obtain Riot user: " << result->status);
        return user;
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    user.username = parsedJson["username"].get<std::string>();
    user.tag = parsedJson["tag"].get<std::string>();
    return user;
}

SquadovApi* getGlobalApi() {
    static SquadovApiPtr global = std::make_unique<SquadovApi>();
    return global.get();
}

}