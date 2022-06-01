#include "api/squadov_api.h"

#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/url.h"
#include "shared/json.h"
#include "shared/filesystem/common_paths.h"

#include <nlohmann/json.hpp>
#include <sstream>
#include <filesystem>

#define DEBUG_REQUEST_BODY_TO_DISK 0

namespace fs = std::filesystem;
namespace service::api {
namespace {

const std::string WEB_SESSION_HEADER_KEY = "x-squadov-session-id";
const std::string WEB_MACHINE_ID_KEY = "x-squadov-machine-id";
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

int64_t SquadovApi::addSessionIdUpdateCallback(const SessionIdUpdateCallback& cb) {
    std::lock_guard guard(_sessionUpdateMutex);
    const auto id = _sessionUpdateCbCounter++;
    _sessionUpdateCallback[id] = cb;
    return id;
}

void SquadovApi::removeSessionIdUpdateCallback(int64_t idx) {
    _sessionUpdateCallback.erase(idx);
}

void SquadovApi::setSessionId(const std::string& key) {
    _webClient->setHeaderKeyValue(WEB_SESSION_HEADER_KEY, key);

    const auto machineIdFname = shared::filesystem::getSquadOvUserFolder() / fs::path(".machineId");
    if (fs::exists(machineIdFname)) {
        std::ifstream f(machineIdFname);
        std::string id;
        f >> id;
        LOG_INFO("Got Machine ID: " << id << std::endl);
        _webClient->setHeaderKeyValue(WEB_MACHINE_ID_KEY, id);
    }

    std::optional<shared::squadov::SquadOVUser> newUser;
    for (int i = 0; i < 5; ++i) {
        try {
            newUser = getCurrentUserApi();
            break;
        } catch (std::exception& ex) {
            LOG_WARNING("...Failed to get current user from API on try : " << i << std::endl);
            newUser = std::nullopt;
        }
    }

    {
        std::unique_lock<std::shared_mutex> guard(_sessionMutex);
        _session.sessionId = key;
        _session.user = newUser;

        if (!_session.user) {
            THROW_ERROR("Failed to get current user from session ID.");
        }
    }

    {
        std::lock_guard guard(_sessionUpdateMutex);
        for (const auto& kvp: _sessionUpdateCallback) {
            kvp.second(key);
        }
    }
}

int64_t SquadovApi::getSessionUserId() const {
    std::shared_lock<std::shared_mutex> guard(_sessionMutex);
    return _session.user.value().id;
}

std::string SquadovApi::getSessionUserUuid() const {
    std::shared_lock<std::shared_mutex> guard(_sessionMutex);
    return _session.user.value().uuid;
}

std::string SquadovApi::getSessionUsername() const {
    std::shared_lock<std::shared_mutex> guard(_sessionMutex);
    return _session.user.value().username;
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

void SquadovApi::createBookmark(const std::string& videoUuid, const shared::TimePoint& tm) const {
    std::ostringstream path;
    path << "/v1/users/me/events";

    nlohmann::json body = {
        { "videoUuid", videoUuid },
        { "tm", shared::timeToIso(tm) }
    };
    const auto result = _webClient->post(path.str(), body);
    if (result->status != 204) {
        THROW_ERROR("Failed to get create bookmark: " << result->status);
        return;
    }
}

void SquadovApi::markUserAnalyticsEvent(const std::string& eventId) const {
    std::ostringstream path;
    path << "/v1/users/me/analytics/event";

    nlohmann::json body = {
        { "eventId", eventId },
        { "platform", "DESKTOP" }
    };
    const auto result = _webClient->post(path.str(), body);
    // THIS SHOULD NEVER BE A FAILURE. We don't care about analytics THAT much.
    if (result->status != 204) {
        LOG_WARNING("Failed to get mark user analytics event: " << result->status);
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
    flags.maxBitrateKbps = parsedJson["maxBitrateKbps"].get<int>();
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
    return _session.user.value();
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

service::uploader::UploadDestination SquadovApi::createVodDestinationUri(const std::string& videoUuid, const std::string& containerFormat) const {
    const std::string path = "/v1/vod";

    const nlohmann::json body = {
        { "videoUuid", videoUuid },
        { "containerFormat", containerFormat }
    };
    const auto result = _webClient->post(path, body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create VOD: " << result->status);
        return service::uploader::UploadDestination{};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return service::uploader::UploadDestination::fromJson(parsedJson);
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

service::uploader::UploadDestination SquadovApi::getObjectPartUploadUri(const std::string& objectUuid, const std::string& bucket, const std::string& session, service::uploader::UploadPurpose uploadPurpose, int64_t part, bool needsAccel) const {
    std::ostringstream path;
    switch(uploadPurpose) {
        case service::uploader::UploadPurpose::VOD:
            path << "/v1/vod/" << objectUuid << "/upload?part=";
            break;
        case service::uploader::UploadPurpose::SpeedCheck:
            path << "/v1/speedcheck/" << objectUuid << "?part=";
            break;
        default:
            THROW_ERROR("Failed to find Upload Purpose.")
            break;
    }
    path << part << "&bucket=" << shared::url::urlEncode(bucket) << "&session=" << session << "&accel=" << needsAccel;
    const auto result = _webClient->get(path.str());
    if (result->status != 200) {
        THROW_ERROR("Failed to get VOD part upload URI: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return service::uploader::UploadDestination::fromJson(parsedJson);
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

service::uploader::UploadDestination SquadovApi::getSpeedCheckUri(const std::string& speedCheckUuid) const {
    std::ostringstream path;
    path << "/v1/speedcheck/" << speedCheckUuid;

    const auto result = _webClient->get(path.str());

    if (result->status != 200) {
        THROW_ERROR("Failed to get Speed Check URI: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return service::uploader::UploadDestination::fromJson(parsedJson);
}

void SquadovApi::postSpeedCheck(const double speedMbps, const std::string& speedCheckUuid) const {
    std::ostringstream path;
    path << "/v1/speedcheck/" << speedCheckUuid;

    const nlohmann::json body = {
        { "speedMbps", speedMbps }
    };

    const auto result = _webClient->post(path.str(), body);

    if (result->status != 204) {
        THROW_ERROR("Failed to post Speed Check to Server: " << result->status);
        return;
    }
}


std::string SquadovApi::getVodMd5Checksum(const std::string& videoUuid) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid << "/source/fastify.mp4?md5=1";

    const auto result = _webClient->get(path.str());

    if (result->status != 200) {
        THROW_ERROR("Failed to get VOD MD5: " << result->status);
        return {};
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::createStagedClip(const std::string& videoUuid, int64_t start, int64_t end) const {
    std::ostringstream path;
    path << "/v1/vod/" << videoUuid << "/stage";

    const nlohmann::json body = {
        { "start", start},
        { "end", end },
        { "execute", false }
    };
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to stage clip: " << result->status);
    }
}

std::string SquadovApi::createWoWChallengeMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeStart& encounter, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", encounter.toJson() },
        { "cl", cl.toJson() },
        { "session", sessionId }
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

std::string SquadovApi::createWoWEncounterMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWEncounterStart& encounter, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", encounter.toJson() },
        { "cl", cl.toJson() },
        { "session", sessionId }
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

std::string SquadovApi::createWoWArenaMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWArenaStart& arena, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", arena.toJson() },
        { "cl", cl.toJson() },
        { "session", sessionId }
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

std::string SquadovApi::createWowInstanceMatch(const shared::TimePoint& timestamp, const shared::wow::TypedInstanceData& data, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", shared::json::JsonConverter<shared::wow::TypedInstanceData>::to(data) },
        { "cl", cl.toJson() },
        { "session", sessionId }
    };

    std::ostringstream path;
    path << "/v1/wow/match/instance";
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 200) {
        THROW_ERROR("Failed to create WoW instance: " << result->status);
        return "";
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

std::string SquadovApi::finishWowInstanceMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants) {
    nlohmann::json combatantArray = nlohmann::json::array();
    for (const auto& c : combatants) {
        combatantArray.push_back(c.toJson());
    }

    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "combatants", combatantArray }
    };

    std::ostringstream path;
    path << "/v1/wow/match/instance/" << matchUuid;

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

    THROW_ERROR("Failed to finish WoW instance: " << lastStatus);
    return "";
}

void SquadovApi::connectWowMatchViewToCombatLog(const std::string& viewUuid, const std::string& combatLogId) {
    std::ostringstream path;
    path << "/v1/wow/view/" << viewUuid << "/cl/" << combatLogId;
    const auto result = _webClient->post(path.str(), nlohmann::json{});

    if (result->status != 204) {
        THROW_ERROR("Failed to link wow match view to combat log: " << result->status);
        return;
    }
}

void SquadovApi::convertWowInstanceViewToKeystone(const std::string& viewUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeStart& keystone, const game_event_watcher::WoWCombatLogState& cl) {
    const nlohmann::json body = {
        { "timestamp", shared::timeToIso(timestamp)},
        { "data", keystone.toJson() },
        { "cl", cl.toJson() }
    };

    std::ostringstream path;
    path << "/v1/wow/match/instance/" << viewUuid << "/convert/keystone";
    const auto result = _webClient->post(path.str(), body);

    if (result->status != 204) {
        THROW_ERROR("Failed to create WoW challenge: " << result->status);
        return;
    }
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

bool SquadovApi::verifyLeagueOfLegendsAccountOwnership(const std::string& summonerName, const std::string& puuid, const std::string& platformId) const {
    {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        if (_verifiedRiotAccounts.find(puuid) != _verifiedRiotAccounts.end()) {
            return true;
        }
    }

    const nlohmann::json body = {
        { "summonerName", summonerName },
        { "puuid", puuid },
        { "platformId", platformId }
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

bool SquadovApi::verifyTftAccountOwnership(const std::string& summonerName, const std::string& puuid, const std::string& platformId) const {
    {
        std::lock_guard guard(_riotAccountOwnershipMutex);
        if (_verifiedRiotAccounts.find(puuid) != _verifiedRiotAccounts.end()) {
            return true;
        }
    }
    
    const nlohmann::json body = {
        { "summonerName", summonerName },
        { "puuid", puuid },
        { "platformId", platformId }
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

    const auto result = _webClient->post(path.str(), nlohmann::json{});
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

    const auto result = _webClient->post(path.str(), nlohmann::json{});
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

AwsCognitoCredentials SquadovApi::getAwsCredentials() const {
    std::ostringstream path;
    path << "/v1/aws/credentials";
    
    const auto result = _webClient->get(path.str());
    if (result->status != 200) {
        THROW_ERROR("Failed to get AWS credentials: " << result->status);
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return shared::json::JsonConverter<AwsCognitoCredentials>::from(parsedJson);
}

std::string SquadovApi::getCombatLogApiConfiguration() const {
    std::ostringstream path;
    path << "/v1/cl/config";
    
    const auto result = _webClient->get(path.str());
    if (result->status != 200) {
        THROW_ERROR("Failed to get combat log api config: " << result->status);
    }

    const auto parsedJson = nlohmann::json::parse(result->body);
    return parsedJson.get<std::string>();
}

void SquadovApi::createCombatLog(const std::string& partitionId, const shared::TimePoint& startTm, const nlohmann::json& clState) const {
    std::ostringstream path;
    path << "/v1/cl/partition/" << partitionId;

    const nlohmann::json data = {
        { "startTime", shared::timeToIso(startTm) },
        { "clState", clState }
    };
    
    const auto result = _webClient->post(path.str(), data);
    if (result->status != 204) {
        THROW_ERROR("Failed to create combat log: " << result->status);
    }
}

void SquadovApi::syncLocalStorage(const std::vector<std::string>& videoUuids) {
    std::ostringstream path;
    path << "/v1/users/me/vod/local/sync";

    nlohmann::json arr = nlohmann::json::array();
    for (const auto& k : videoUuids) {
        arr.push_back(k);
    }

    const auto result = _webClient->post(path.str(), arr);
    if (result->status != 204) {
        THROW_ERROR("Failed to sync local storage: " << result->status);
    }
}

void SquadovApi::addLocalStorage(const std::string& videoUuid) {
    std::ostringstream path;
    path << "/v1/users/me/vod/local/" << videoUuid;

    const auto result = _webClient->post(path.str(), nlohmann::json{});
    if (result->status != 204) {
        THROW_ERROR("Failed to add to local storage: " << result->status);
    }
}

void SquadovApi::removeLocalStorage(const std::string& videoUuid) {
    std::ostringstream path;
    path << "/v1/users/me/vod/local/" << videoUuid;

    const auto result = _webClient->del(path.str());
    if (result->status != 204) {
        THROW_ERROR("Failed to remove from local storage: " << result->status);
    }
}

SquadovApi* getGlobalApi() {
    static SquadovApiPtr global = std::make_unique<SquadovApi>();
    return global.get();
}

}