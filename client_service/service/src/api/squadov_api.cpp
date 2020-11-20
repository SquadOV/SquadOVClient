#include "api/squadov_api.h"

#include "shared/env.h"
#include "shared/errors/error.h"

#include <nlohmann/json.hpp>
#include <sstream>

#define DEBUG_REQUEST_BODY_TO_DISK 0

namespace service::api {
namespace {

const std::string WEB_SESSION_HEADER_KEY = "x-squadov-session-id";
const std::string SET_SESSION_HEADER_KEY = "x-squadov-set-session-id";

}

SquadovApi::SquadovApi() {
    {
        // TODO: Need some better way of communicating this OR just removing it altogether and not
        // having this reliance.
        std::ostringstream host;
        host << "https://127.0.0.1:" << shared::getEnv("SQUADOV_API_PORT", "50000");
        _localClient = std::make_unique<http::HttpClient>(host.str());
        _localClient->setBearerAuthToken(shared::getEnv("SQUADOV_API_KEY", "AAAAA"));
        _localClient->enableSelfSigned();
    }

    {
        std::ostringstream host;
        host << shared::getEnv("API_SQUADOV_URL", "http://127.0.0.1:8080");
        _webClient = std::make_unique<http::HttpClient>(host.str());
        _webClient->addResponseInterceptor([this](service::http::HttpResponse& response){
            if (!_sessionUpdateCallback) {
                return true;
            }
            for (const auto& [key, value] : response.headers) {
                if (key == SET_SESSION_HEADER_KEY) {
                    _sessionUpdateCallback(value);
                    break;
                }
            }
            return true;
        });

        // Some backfill tasks might get pretty heavy so put in a rate limit here.
        // For reference, this is 5 requests per second -- which will translate to about 500 aimlab tasks synced per second
        // which should be plenty fast.
        _webClient->setRateLimit(0.2);
    }
}

void SquadovApi::setSessionId(const std::string& key) {
    _webClient->setHeaderKeyValue(WEB_SESSION_HEADER_KEY, key);
    _session.sessionId = key;
    _session.user = getCurrentUser();
}

shared::squadov::SquadOVUser SquadovApi::getCurrentUser() const {
    const std::string path = "/v1/users/me/profile";

    const auto result = _webClient->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get SquadOV user: " << result->status);
        return shared::squadov::SquadOVUser{};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);

    shared::squadov::SquadOVUser ret;
    ret.id = parsedJson["id"].get<int64_t>();
    ret.username = parsedJson["username"].get<std::string>();
    ret.email = parsedJson["email"].get<std::string>();
    ret.verified = parsedJson["verified"].get<bool>();
    ret.uuid = parsedJson["uuid"].get<std::string>();
    return ret;
}

shared::riot::RiotRsoToken SquadovApi::refreshValorantRsoToken(const std::string& puuid) const {
    shared::riot::RiotRsoToken token;
    token.puuid = puuid;

    std::ostringstream path;
    path << "/valorant/accounts/" << puuid << "/rso";

    const auto result = _localClient->get(path.str().c_str());

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

    const auto result = _localClient->get(path.str().c_str());

    if (result->status != 200) {
        THROW_ERROR("Failed to obtain Riot user: " << result->status);
        return user;
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    user.username = parsedJson["username"].get<std::string>();
    user.tag = parsedJson["tag"].get<std::string>();
    return user;
}

shared::riot::RiotUser SquadovApi::updateRiotUsername(const std::string& puuid, const std::string& username, const std::string& tagline) const {
    shared::riot::RiotUser user;
    user.puuid = puuid;
    user.username = username;
    user.tag = tagline;

    std::ostringstream path;
    path << "/valorant/accounts/" << puuid << "/username";

    nlohmann::json body;
    body["username"] = username;
    body["tag"] = tagline;
    const auto result = _localClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to update Riot username: " << result->status);
        return user;
    }
    
    return user;
}

std::string SquadovApi::uploadValorantMatch(const std::string& matchId,  const nlohmann::json& rawData, const nlohmann::json& playerData) const {
    const std::string path = "/v1/valorant";
    nlohmann::json body = {
        { "matchId", matchId },
    };

    if (!rawData.is_null()) {
        body["rawData"] = rawData.dump();
    }

    if (!playerData.is_null()) {
        body["playerData"] = playerData;
    }

    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to upload Valorant match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson["matchUuid"].get<std::string>();
}

std::vector<std::string> SquadovApi::obtainMissingValorantMatches(const std::vector<std::string>& ids) const {
    const std::string path = "/v1/valorant/backfill";
    nlohmann::json body = nlohmann::json::array();
    for (const auto& id : ids ) {
        body.push_back(id);
    }

    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to obtain matches to backfill: " << result->status);
        return std::vector<std::string>();
    }

    std::vector<std::string> ret;
    const auto parsedJson = nlohmann::json::parse(result->body);
    for (const auto& v : parsedJson) {
        ret.push_back(v);
    }    
    return ret;
}

std::string SquadovApi::uploadAimlabTask(const shared::aimlab::TaskData& data) const {
    const std::string path = "/v1/aimlab";
    const nlohmann::json body = data.toJson();
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to upload Aim Lab task: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson["matchUuid"].get<std::string>();
}

void SquadovApi::bulkUploadAimlabTasks(const std::vector<shared::aimlab::TaskData>& data) const {
    const std::string path = "/v1/aimlab/bulk";
    nlohmann::json body = nlohmann::json::array();
    for (const auto& d : data) {
        body.emplace_back(d.toJson());
    }

    const auto result = _webClient->post(path, body);
    if (result->status != 200) {
        THROW_ERROR("Failed to bulk upload Aim Lab tasks: " << result->status);
        return;
    }
}

std::string SquadovApi::createHearthstoneMatch(
    const game_event_watcher::HearthstoneGameConnectionInfo& info,
    const process_watcher::memory::games::hearthstone::types::CollectionDeckMapper& deck,
    const std::unordered_map<int, process_watcher::memory::games::hearthstone::types::PlayerMapperSPtr>& players,
    const shared::TimePoint& timestamp
) const {
    nlohmann::json body = {
        { "info", info.toJson() },
        { "deck", deck.toJson() },
        { "timestamp", shared::timeToIso(timestamp) }
    };

    nlohmann::json jPlayers;
    for (const auto& kvp : players) {
        jPlayers[std::to_string(kvp.first)] = kvp.second->toJson();
    }
    body["players"] = jPlayers;

#if DEBUG_REQUEST_BODY_TO_DISK
    {
        std::ofstream dbg("createHearthstoneMatch.json");
        dbg << body.dump(4);
    }
#endif

    const std::string path = "/v1/hearthstone/match";
    const auto result = _webClient->post(path, body);
    if (result->status != 200) {
        THROW_ERROR("Failed to create Hearthstone match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::uploadHearthstonePowerLogs(
    const std::string& matchUuid,
    const nlohmann::json& logs
) const {
    std::ostringstream path;
    path << "/v1/hearthstone/match/" << matchUuid;

#if DEBUG_REQUEST_BODY_TO_DISK
    {
        std::ofstream dbg("uploadHearthstonePowerLogs.json");
        dbg << logs.dump(4);
    }
#endif

    const auto result = _webClient->post(path.str(), logs);
    if (result->status != 200) {
        THROW_ERROR("Failed to upload Hearthstone match power logs: " << result->status);
    }
}

std::string SquadovApi::createHearthstoneArenaDraft(const shared::TimePoint& timestamp, int64_t deckId) const {
    std::ostringstream path;
    path << "/v1/hearthstone/user/" << _session.user.id << "/arena";

    const nlohmann::json body = {
        { "deckId", deckId },
        { "tm", shared::timeToIso(timestamp) }
    };

    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create Hearthstone draft: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::addHearthstoneArenaDraftCard(const shared::TimePoint& timestamp, const std::string& arenaUuid, const std::string& cardId) const {
    std::ostringstream path;
    path << "/v1/hearthstone/user/" << _session.user.id << "/arena/" << arenaUuid;

    const nlohmann::json body = {
        { "cardId", cardId },
        { "tm", shared::timeToIso(timestamp) }
    };

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to add Hearthstone draft card: " << result->status);
    }
}

void SquadovApi::uploadHearthstoneArenaDeck(const process_watcher::memory::games::hearthstone::types::CollectionDeckMapper& deck, const std::string& arenaUuid) const {
    std::ostringstream path;
    path << "/v1/hearthstone/user/" << _session.user.id << "/arena/" << arenaUuid << "/deck";

    const auto result = _webClient->post(path.str(), deck.toJson());
    if (result->status != 200) {
        THROW_ERROR("Failed to upload Hearthstone final draft deck: " << result->status);
    }
}

std::string SquadovApi::createVodDestinationUri(const std::string& videoUuid) const {
    const std::string path = "/v1/vod";

    const nlohmann::json body = {
        { "videoUuid", videoUuid },
    };
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to associate VOD: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::associateVod(const shared::squadov::VodAssociation& association, const shared::squadov::VodMetadata& metadata) const {
    std::ostringstream path;
    path << "/v1/vod/" << association.videoUuid;

    const nlohmann::json body = {
        { "association", association.toJson() },
        { "metadata", metadata.toJson() },
    };
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to associate VOD: " << result->status);
        return;
    }
}

void SquadovApi::deleteVod(const std::string& videoUuid) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid;

    const auto result = _webClient->del(path.str());

    if (result->status != 200) {
        THROW_ERROR("Failed to delete VOD: " << result->status);
        return;
    }
}

SquadovApi* getGlobalApi() {
    static SquadovApiPtr global = std::make_unique<SquadovApi>();
    return global.get();
}

}