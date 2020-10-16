#include "api/squadov_api.h"

#include "shared/env.h"
#include "shared/errors/error.h"

#include <nlohmann/json.hpp>
#include <sstream>

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
        host << shared::getEnv("API_URL", "http://127.0.0.1:8080");
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

std::string SquadovApi::uploadValorantMatch(const std::string& matchId, const std::string& rawData) const {
    const std::string path = "/v1/valorant";
    const nlohmann::json body = {
        { "matchId", matchId },
        { "rawData", rawData }
    };
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to upload Valorant match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson["matchUuid"].get<std::string>();
}

std::string SquadovApi::uploadAimlabTask(const shared::aimlab::TaskData& data) const {
    const std::string path = "/v1/aimlab";
    const nlohmann::json body = {
        { "id", data.taskId },
        { "klutchId", data.klutchId },
        { "taskName", data.taskName },
        { "mode", data.mode },
        { "score", data.score },
        { "version", data.version },
        { "createDate", shared::timeToIso(data.createDate) },
        { "rawData", data.rawData }
    };
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to upload Aim Lab task: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson["matchUuid"].get<std::string>();
}

void SquadovApi::associateVod(const shared::squadov::VodAssociation& association) const {
    const std::string path = "/v1/vod";

    const nlohmann::json body = {
        { "matchUuid", association.matchUuid },
        { "userUuid", association.userUuid },
        { "vodUuid", association.videoUuid },
        { "startTime", shared::timeToIso(association.startTime) },
        { "endTime", shared::timeToIso(association.endTime  ) },
    };
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to associate VOD: " << result->status);
        return;
    }
}

void SquadovApi::deleteVod(const std::string& vodUuid) const {
    std::ostringstream path;
    path << "/v1/vod/" << vodUuid;

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