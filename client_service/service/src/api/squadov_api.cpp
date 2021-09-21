#include "api/squadov_api.h"

#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/url.h"

#include <nlohmann/json.hpp>
#include <sstream>

#define DEBUG_REQUEST_BODY_TO_DISK 0

namespace service::api {
namespace {

const std::string WEB_SESSION_HEADER_KEY = "x-squadov-session-id";
constexpr int MAX_WOW_MATCH_FINISH_RETRY = 3;

}

SquadovApi::SquadovApi():
    _rdGen(std::random_device{}()),
    _rdDist(0.0, 1.0)
{
    {
        std::ostringstream host;
        host << shared::getEnv("API_SQUADOV_URL", "http://127.0.0.1:8080");
        _webClient = std::make_unique<shared::http::HttpClient>(host.str());

        // Some backfill tasks might get pretty heavy so put in a rate limit here.
        // For reference, this is 5 requests per second -- which will translate to about 500 aimlab tasks synced per second
        // which should be plenty fast.
        _webClient->setRateLimit(0.2);
    }
}

std::chrono::milliseconds SquadovApi::generateRandomBackoff(int64_t base, size_t tryCount) const {
    std::lock_guard guard(_randomMutex);
    return std::chrono::milliseconds(static_cast<int64_t>(base + std::pow(2, tryCount) + _rdDist(_rdGen) * 1000));
}

KafkaInfo SquadovApi::getKafkaInfo() const {
    const std::string path = "/v1/kafka/info";

    const auto result = _webClient->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get SquadOV Kafka info: " << result->status);
        return KafkaInfo{};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);

    KafkaInfo ret;
    ret.servers = parsedJson["servers"].get<std::string>();
    ret.key = parsedJson["key"].get<std::string>();
    ret.secret = parsedJson["secret"].get<std::string>();
    ret.wowTopic = parsedJson["wowTopic"].get<std::string>();
    return ret;
}

std::string SquadovApi::getSentryDsn() const {
    const std::string path = "/v1/sentry/desktop";

    const auto result = _webClient->get(path);

    if (result->status != 200) {
        THROW_ERROR("Failed to get SquadOV client service sentry info: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::setSessionId(const std::string& key) {
    _webClient->setHeaderKeyValue(WEB_SESSION_HEADER_KEY, key);

    const auto user = getCurrentUserApi();

    std::unique_lock<std::shared_mutex> guard(_sessionMutex);
    _session.sessionId = key;
    _session.user = user;
}

int64_t SquadovApi::getSessionUserId() const {
    std::shared_lock<std::shared_mutex> guard(_sessionMutex);
    return _session.user.id;
}

std::string SquadovApi::getSessionUserUuid() const {
    std::shared_lock<std::shared_mutex> guard(_sessionMutex);
    return _session.user.uuid;
}

std::string SquadovApi::getSessionUsername() const {
    std::shared_lock<std::shared_mutex> guard(_sessionMutex);
    return _session.user.username;
}

void SquadovApi::syncHardware(const service::hardware::Hardware& data) const {
    std::ostringstream path;
    path << "/v1/users/me/hw";

    const auto result = _webClient->post(path.str(), data.toJson());

    if (result->status != 204) {
        THROW_ERROR("Failed to get sync specs: " << result->status);
        return;
    }
}

void SquadovApi::retrieveSessionFeatureFlags() {
    std::ostringstream path;
    path << "/v1/users/" << getSessionUserId() << "/features";

    const auto result = _webClient->get(path.str());

    if (result->status != 200) {
        THROW_ERROR("Failed to get SquadOV features: " << result->status);
        return;
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    shared::squadov::FeatureFlags flags;
    flags.maxRecordPixelY = parsedJson["maxRecordPixelY"].get<int>();
    flags.maxRecordFps = parsedJson["maxRecordFps"].get<int>();
    flags.allowRecordUpload = parsedJson["allowRecordUpload"].get<bool>();
    flags.allowWowCombatLogUpload = parsedJson["allowWowCombatLogUpload"].get<bool>();
    flags.disableSentry = parsedJson.value("disableSentry", true);
    LOG_INFO("Obtain feature flags: " << flags << std::endl);
    
    std::lock_guard guard(_featureMutex);
    _features = flags;
}

shared::squadov::SquadOVUser SquadovApi::getCurrentUserApi() const {
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

shared::squadov::SquadOVUser SquadovApi::getCurrentUserCached() const {
    std::shared_lock guard(_sessionMutex);
    return _session.user;
}

std::string SquadovApi::uploadValorantMatch(const std::string& matchId, const std::string& puuid, const nlohmann::json& playerData) const {
    const std::string path = "/v1/valorant";
    nlohmann::json body = {
        { "matchId", matchId },
        { "puuid", puuid },
        { "playerData", playerData } 
    };

    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to upload Valorant match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::requestValorantMatchBackfill(const std::string& puuid) const {
    std::ostringstream path;
    path << "/v1/valorant/user/" << getSessionUserId() << "/backfill";

    const nlohmann::json body = {
        { "puuid", puuid }
    };

    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to start match backfill: " << result->status);
        return;
    }
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
    const process_watcher::memory::games::hearthstone::types::CollectionDeckMapperSPtr& deck,
    const std::unordered_map<int, process_watcher::memory::games::hearthstone::types::PlayerMapperSPtr>& players,
    const shared::hearthstone::HearthstoneRatings& ratings,
    const shared::TimePoint& timestamp
) const {
    nlohmann::json body = {
        { "info", info.toJson() },
        { "deck", deck ? deck->toJson() : nlohmann::json() },
        { "timestamp", shared::timeToIso(timestamp) }
    };

    nlohmann::json jPlayers;
    for (const auto& kvp : players) {
        LOG_INFO("Serializing player: " << kvp.second->name() << "[" << kvp.first << "]" << std::endl);
        auto j = kvp.second->toJson();

        // This should be fine as everything within ratings is optional within
        // the player structure server-side.
        if (kvp.second->local()) {
            j.update(ratings.toJson());
        }

        jPlayers[std::to_string(kvp.first)] = j;
    }
    body["players"] = jPlayers;

#if DEBUG_REQUEST_BODY_TO_DISK
    {
        std::ofstream dbg("createHearthstoneMatch.json");
        dbg << body.dump(4);
    }
#endif

    std::ostringstream path;
    path << "/v1/hearthstone/user/" << getCurrentUserCached().id << "/match";

    const auto result = _webClient->post(path.str(), body);
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
    path << "/v1/hearthstone/user/" << getCurrentUserCached().id << "/match/" << matchUuid;

#if DEBUG_REQUEST_BODY_TO_DISK
    {
        std::ofstream dbg("uploadHearthstonePowerLogs.json");
        dbg << logs.dump(4);
    }
#endif

    const auto result = _webClient->post(path.str(), logs, true);
    if (result->status != 200) {
        THROW_ERROR("Failed to upload Hearthstone match power logs: " << result->status);
    }
}

std::string SquadovApi::createHearthstoneArenaDraft(const shared::TimePoint& timestamp, int64_t deckId) const {
    std::ostringstream path;
    path << "/v1/hearthstone/user/" << getSessionUserId() << "/arena";

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
    path << "/v1/hearthstone/user/" << getSessionUserId() << "/arena/" << arenaUuid;

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
    path << "/v1/hearthstone/user/" << getSessionUserId() << "/arena/" << arenaUuid << "/deck";

    const auto result = _webClient->post(path.str(), deck.toJson());
    if (result->status != 200) {
        THROW_ERROR("Failed to upload Hearthstone final draft deck: " << result->status);
    }
}

service::vod::VodDestination SquadovApi::createVodDestinationUri(const std::string& videoUuid, const std::string& containerFormat) const {
    const std::string path = "/v1/vod";

    const nlohmann::json body = {
        { "videoUuid", videoUuid },
        { "containerFormat", containerFormat }
    };
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create VOD: " << result->status);
        return service::vod::VodDestination{};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return service::vod::VodDestination::fromJson(parsedJson);
}

void SquadovApi::associateVod(const shared::squadov::VodAssociation& association, const shared::squadov::VodMetadata& metadata, const std::string& sessionUri, const std::vector<std::string>& parts) const {
    std::ostringstream path;
    path << "/v1/vod/" << association.videoUuid;

    nlohmann::json body = {
        { "association", association.toJson() },
        { "metadata", metadata.toJson() },
        { "parts", nlohmann::json::array() }
    };

    if (sessionUri.empty()) {
        body["sessionUri"] = nlohmann::json();
    } else {
        body["sessionUri"] = sessionUri;
    }

    for (const auto& p : parts) {
        body["parts"].push_back(p);
    }

    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to associate VOD: " << result->status);
        return;
    }
}

service::vod::VodDestination SquadovApi::getVodPartUploadUri(const std::string& videoUuid, const std::string& bucket, const std::string& session, int64_t part) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid << "/upload?part=" << part << "&bucket=" << shared::url::urlEncode(bucket) << "&session=" << session;

    const auto result = _webClient->get(path.str());
    if (result->status != 200) {
        THROW_ERROR("Failed to get VOD part upload URI: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return service::vod::VodDestination::fromJson(parsedJson);
}

void SquadovApi::deleteVod(const std::string& videoUuid) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid;

    const auto result = _webClient->del(path.str());

    if (result->status != 204) {
        THROW_ERROR("Failed to delete VOD: " << result->status);
        return;
    }
}

shared::squadov::VodAssociation SquadovApi::getVod(const std::string& videoUuid) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid << "/assoc";

    const auto result = _webClient->get(path.str());

    if (result->status != 200) {
        THROW_ERROR("Failed to get VOD: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return shared::squadov::VodAssociation::fromJson(parsedJson);
}

std::string SquadovApi::getVodUri(const std::string& videoUuid) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid << "/source/fastify.mp4";

    const auto result = _webClient->get(path.str());

    if (result->status != 200) {
        THROW_ERROR("Failed to get VOD URI: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::obtainNewWoWCombatLogUuid(const game_event_watcher::WoWCombatLogState& log) const {
    std::ostringstream path;
    path << "/v1/wow/combatlog";
    const auto result = _webClient->post(path.str(), log.toJson());

    if (result->status != 200) {
        THROW_ERROR("Failed to create WoW combat log: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::createWoWChallengeMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeStart& encounter, const game_event_watcher::WoWCombatLogState& cl) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", encounter.toJson() },
        { "cl", cl.toJson() }
    };

    std::ostringstream path;
    path << "/v1/wow/match/challenge";
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create WoW challenge: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::finishWoWChallengeMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeEnd& encounter, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants) {
    nlohmann::json combatantArray = nlohmann::json::array();
    for (const auto& c : combatants) {
        combatantArray.push_back(c.toJson());
    }

    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", encounter.toJson() },
        { "combatants", combatantArray }
    };

    std::ostringstream path;
    path << "/v1/wow/match/challenge/" << matchUuid;

    int lastStatus = 0;
    for (int i = 0; i < MAX_WOW_MATCH_FINISH_RETRY; ++i) {
        const auto result = _webClient->post(path.str(), body);

        lastStatus = result->status;
        if (result->status != 200) {
            LOG_INFO("...Error from server...retrying." << std::endl);
            std::this_thread::sleep_for(generateRandomBackoff(1000, static_cast<size_t>(i)));
            continue;
        }

        const auto parsedJson = nlohmann::json::parse(result->body);
        return parsedJson.get<std::string>();
    }

    THROW_ERROR("Failed to finish WoW challenge: " << lastStatus);
    return "";
}

std::string SquadovApi::createWoWEncounterMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWEncounterStart& encounter, const game_event_watcher::WoWCombatLogState& cl) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", encounter.toJson() },
        { "cl", cl.toJson() }
    };

    std::ostringstream path;
    path << "/v1/wow/match/encounter";
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create WoW encounter: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::finishWoWEncounterMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWEncounterEnd& encounter, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants) {
    nlohmann::json combatantArray = nlohmann::json::array();
    for (const auto& c : combatants) {
        combatantArray.push_back(c.toJson());
    }

    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", encounter.toJson() },
        { "combatants", combatantArray }
    };

    std::ostringstream path;
    path << "/v1/wow/match/encounter/" << matchUuid;

    int lastStatus = 0;
    for (int i = 0; i < MAX_WOW_MATCH_FINISH_RETRY; ++i) {
        const auto result = _webClient->post(path.str(), body);

        lastStatus = result->status;
        if (result->status != 200) {
            LOG_INFO("...Error from server...retrying." << std::endl);
            std::this_thread::sleep_for(generateRandomBackoff(1000, static_cast<size_t>(i)));
            continue;
        }

        const auto parsedJson = nlohmann::json::parse(result->body);
        return parsedJson.get<std::string>();
    }

    THROW_ERROR("Failed to finish WoW encounter: " << lastStatus);
    return "";
}

std::string SquadovApi::createWoWArenaMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWArenaStart& arena, const game_event_watcher::WoWCombatLogState& cl) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", arena.toJson() },
        { "cl", cl.toJson() }
    };

    std::ostringstream path;
    path << "/v1/wow/match/arena";
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create WoW arena: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::finishWoWArenaMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWArenaEnd& arena, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants) {
    nlohmann::json combatantArray = nlohmann::json::array();
    for (const auto& c : combatants) {
        combatantArray.push_back(c.toJson());
    }

    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", arena.toJson() },
        { "combatants", combatantArray }
    };

    std::ostringstream path;
    path << "/v1/wow/match/arena/" << matchUuid;

    int lastStatus = 0;
    for (int i = 0; i < MAX_WOW_MATCH_FINISH_RETRY; ++i) {
        const auto result = _webClient->post(path.str(), body);

        lastStatus = result->status;
        if (result->status != 200) {
            LOG_INFO("...Error from server...retrying." << std::endl);
            std::this_thread::sleep_for(generateRandomBackoff(1000, static_cast<size_t>(i)));
            continue;
        }

        const auto parsedJson = nlohmann::json::parse(result->body);
        return parsedJson.get<std::string>();
    }

    THROW_ERROR("Failed to finish WoW arena: " << lastStatus);
    return "";
}

bool SquadovApi::verifyValorantAccountOwnership(const std::string& gameName, const std::string& tagLine, const std::string& puuid) const {
    {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        if (_verifiedRiotAccounts.find(puuid) != _verifiedRiotAccounts.end()) {
            return true;
        }
    }

    const nlohmann::json body = {
        { "gameName", gameName },
        { "tagLine", tagLine },
        { "puuid", puuid }
    };

    std::ostringstream path;
    path << "/v1/users/" << getSessionUserId() << "/accounts/riot/valorant/account";

    const auto result = _webClient->post(path.str(), body);
    const bool ok = result->status == 204;
    if (ok) {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        _verifiedRiotAccounts.insert(puuid);
    }
    return ok;
}

bool SquadovApi::verifyLeagueOfLegendsAccountOwnership(const std::string& summonerName, const std::string& puuid) const {
    {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        if (_verifiedRiotAccounts.find(puuid) != _verifiedRiotAccounts.end()) {
            return true;
        }
    }

    const nlohmann::json body = {
        { "summonerName", summonerName },
        { "puuid", puuid }
    };

    std::ostringstream path;
    path << "/v1/users/" << getSessionUserId() << "/accounts/riot/lol/account";

    const auto result = _webClient->post(path.str(), body);
    const bool ok = result->status == 204;
    if (ok) {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        _verifiedRiotAccounts.insert(puuid);
    }
    return ok;
}

bool SquadovApi::verifyTftAccountOwnership(const std::string& summonerName, const std::string& puuid) const {
    {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        if (_verifiedRiotAccounts.find(puuid) != _verifiedRiotAccounts.end()) {
            return true;
        }
    }
    
    const nlohmann::json body = {
        { "summonerName", summonerName },
        { "puuid", puuid }
    };

    std::ostringstream path;
    path << "/v1/users/" << getSessionUserId() << "/accounts/riot/tft/account";

    const auto result = _webClient->post(path.str(), body);
    const bool ok = result->status == 204;
    if (ok) {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        _verifiedRiotAccounts.insert(puuid);
    }
    return ok;
}

std::string SquadovApi::createNewLeagueOfLegendsMatch(const std::string& platform, int64_t matchId, const shared::TimePoint& gameStartTime) const {
    const nlohmann::json body = {
        { "platform", platform },
        { "matchId", matchId },
        { "gameStartTime", shared::timeToIso(gameStartTime) }
    };

    std::ostringstream path;
    path << "/v1/lol";

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to create LoL match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::finishLeagueOfLegendsMatch(const std::string& matchUuid) const {
    std::ostringstream path;
    path << "/v1/lol/match/" << matchUuid << "/finish";

    const auto result = _webClient->post(path.str(), {});
    if (result->status != 200) {
        THROW_ERROR("Failed to finish LoL match: " << result->status);
        return;
    }
}

void SquadovApi::requestLeagueOfLegendsBackfill(const std::string& summonerName, const std::string& platform) const {
    const nlohmann::json body = {
        { "summonerName", summonerName },
        { "platform", platform }
    };

    std::ostringstream path;
    path << "/v1/lol/user/" << getSessionUserId() << "/backfill";

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to request LoL backfill: " << result->status);
        return;
    }
}

std::string SquadovApi::createNewTftMatch(const std::string& region, const std::string& platform, int64_t matchId, const shared::TimePoint& gameStartTime) const {
    const nlohmann::json body = {
        { "platform", platform },
        { "region", region },
        { "matchId", matchId },
        { "gameStartTime", shared::timeToIso(gameStartTime) }
    };

    std::ostringstream path;
    path << "/v1/tft";

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to create TFT match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::finishTftMatch(const std::string& matchUuid) const {
    std::ostringstream path;
    path << "/v1/tft/match/" << matchUuid << "/finish";

    const auto result = _webClient->post(path.str(), {});
    if (result->status != 200) {
        THROW_ERROR("Failed to finish TFT match: " << result->status);
        return;
    }
}

void SquadovApi::requestTftBackfill(const std::string& summonerName, const std::string& region) const {
    const nlohmann::json body = {
        { "summonerName", summonerName },
        { "region", region }
    };

    std::ostringstream path;
    path << "/v1/tft/user/" << getSessionUserId() << "/backfill";

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to request TFT backfill: " << result->status);
        return;
    }
}

std::string SquadovApi::createNewCsgoMatch(const std::string& server, const shared::TimePoint& gameStartTime, const std::string& map, const std::string& mode) {
    const nlohmann::json body = {
        { "server", server },
        { "startTime", shared::timeToIso(gameStartTime) },
        { "map", map },
        { "mode", mode }
    };

    std::ostringstream path;
    path << "/v1/csgo/user/" << getCurrentUserCached().id << "/view";

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to create CSGO match: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::finishCsgoMatch(const std::string& viewUuid, const std::string& localSteamId, const shared::TimePoint& gameStopTime, const game_event_watcher::CsgoMatchState& state, const std::optional<std::string>& demoUrl, const std::optional<shared::TimePoint>& demoTimestamp) {
    nlohmann::json body = {
        { "stopTime", shared::timeToIso(gameStopTime) },
        { "data", shared::json::JsonConverter<game_event_watcher::CsgoMatchState>::to(state) },
        { "localSteamId", std::stoll(localSteamId) }
    };

    if (demoUrl && demoTimestamp) {
        body["demo"] = demoUrl.value();
        body["demoTimestamp"] = shared::timeToIso(demoTimestamp.value());
    }

    std::ostringstream path;
    path << "/v1/csgo/user/" << getCurrentUserCached().id << "/view/" << viewUuid;

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 200) {
        THROW_ERROR("Failed to finalize CSGO match: " << result->status);
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::associateCsgoDemo(const std::string& viewUuid, const std::string& demoUrl, const shared::TimePoint& demoTimestamp) {
    nlohmann::json body = {
        { "demo", demoUrl },
        { "demoTimestamp", shared::timeToIso(demoTimestamp) }
    };

    std::ostringstream path;
    path << "/v1/csgo/user/" << getCurrentUserCached().id << "/view/" << viewUuid << "/demo";

    const auto result = _webClient->post(path.str(), body);
    if (result->status != 204) {
        THROW_ERROR("Failed to associate CSGO demo: " << result->status);
    }
}

SquadovApi* getGlobalApi() {
    static SquadovApiPtr global = std::make_unique<SquadovApi>();
    return global.get();
}

}