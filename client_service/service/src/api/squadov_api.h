#pragma once

#include "shared/http/http_client.h"
#include "shared/riot/riot.h"
#include "shared/squadov/session.h"
#include "shared/squadov/vod.h"
#include "shared/aimlab/aimlab.h"
#include "shared/hearthstone/hearthstone_ratings.h"
#include "shared/wow/instances.h"
#include "shared/time.h"
#include "api/aws_api.h"
#include "uploader/uploader.h"
#include "hardware/hardware.h"

#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "game_event_watcher/wow/wow_log_watcher.h"
#include "game_event_watcher/csgo/csgo_gsi_state_manager.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/player_mapper.h"
#include "shared/squadov/features.h"

#include <functional>
#include <memory>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace service::api {

using SessionIdUpdateCallback = std::function<void(const std::string&)>;

// A mix between our local API (for however much longer we need to keep it around for)
// and our we bapi.
class SquadovApi {
public:
    SquadovApi();

    int64_t addSessionIdUpdateCallback(const SessionIdUpdateCallback& cb);
    void removeSessionIdUpdateCallback(int64_t idx);
    void setSessionId(const std::string& key);

    int64_t getSessionUserId() const;
    std::string getSessionUserUuid() const;
    std::string getSessionUsername() const;
    shared::squadov::FeatureFlags getSessionFeatures() const {
        std::shared_lock guard(_featureMutex);
        return _features;
    }
    void retrieveSessionFeatureFlags();
    shared::TimePoint getServerTime() const;

    // Sentry
    std::string getSentryDsn() const;

    // User
    shared::squadov::SquadOVUser getCurrentUserApi() const;
    shared::squadov::SquadOVUser getCurrentUserCached() const;
    void syncHardware(const service::hardware::Hardware& data) const;
    void createBookmark(const std::string& videoUuid, const shared::TimePoint& tm) const;
    void markUserAnalyticsEvent(const std::string& eventId) const;

    // Local storage
    void syncLocalStorage(const std::vector<std::string>& videoUuids);
    void addLocalStorage(const std::string& videoUuid);
    void removeLocalStorage(const std::string& videoUuid);

    // Valorant
    std::string uploadValorantMatch(const std::string& matchId, const std::string& puuid, const nlohmann::json& playerData) const;
    void requestValorantMatchBackfill(const std::string& puuid) const;
    bool verifyValorantAccountOwnership(const std::string& gameName, const std::string& tagLine, const std::string& puuid) const;

    // Aim Lab
    std::string uploadAimlabTask(const shared::aimlab::TaskData& data) const;
    void bulkUploadAimlabTasks(const std::vector<shared::aimlab::TaskData>& data) const;

    // Hearthstone
    std::string createHearthstoneMatch(
        const game_event_watcher::HearthstoneGameConnectionInfo& info,
        const process_watcher::memory::games::hearthstone::types::CollectionDeckMapperSPtr& deck,
        const std::unordered_map<int, process_watcher::memory::games::hearthstone::types::PlayerMapperSPtr>& players,
        const shared::hearthstone::HearthstoneRatings& ratings,
        const shared::TimePoint& timestamp
    ) const;
    void uploadHearthstonePowerLogs(const std::string& matchUuid, const nlohmann::json& logs) const;
    std::string createHearthstoneArenaDraft(const shared::TimePoint& timestamp, int64_t deckId) const;
    void addHearthstoneArenaDraftCard(const shared::TimePoint& timestamp, const std::string& arenaUuid, const std::string& cardId) const;
    void uploadHearthstoneArenaDeck(const process_watcher::memory::games::hearthstone::types::CollectionDeckMapper& deck, const std::string& arenaUuid) const;

    // WoW
    std::string createWoWChallengeMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeStart& encounter, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId);
    std::string finishWoWChallengeMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeEnd& encounter, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants);
    std::string createWoWEncounterMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWEncounterStart& encounter, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId);
    std::string finishWoWEncounterMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWEncounterEnd& encounter, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants);
    std::string createWoWArenaMatch(const shared::TimePoint& timestamp, const game_event_watcher::WoWArenaStart& arena, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId);
    std::string finishWoWArenaMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWArenaEnd& arena, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants);
    std::string createWowInstanceMatch(const shared::TimePoint& timestamp, const shared::wow::TypedInstanceData& data, const game_event_watcher::WoWCombatLogState& cl, const std::string& sessionId);
    std::string finishWowInstanceMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants);

    void convertWowInstanceViewToKeystone(const std::string& viewUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeStart& keystone, const game_event_watcher::WoWCombatLogState& cl);
    void connectWowMatchViewToCombatLog(const std::string& viewUuid, const std::string& combatLogId);

    // League of Legends
    std::string createNewLeagueOfLegendsMatch(const std::string& platform, int64_t matchId, const shared::TimePoint& gameStartTime) const;
    void finishLeagueOfLegendsMatch(const std::string& matchUuid) const;
    bool verifyLeagueOfLegendsAccountOwnership(const std::string& summonerName, const std::string& puuid, const std::string& platformId) const;
    void requestLeagueOfLegendsBackfill(const std::string& summonerName, const std::string& platform) const;

    // TFT
    std::string createNewTftMatch(const std::string& region, const std::string& platform, int64_t matchId, const shared::TimePoint& gameStartTime) const;
    void finishTftMatch(const std::string& matchUuid) const;
    bool verifyTftAccountOwnership(const std::string& summonerName, const std::string& puuid, const std::string& platformId) const;
    void requestTftBackfill(const std::string& summonerName, const std::string& region) const;

    // CS:GO
    std::string createNewCsgoMatch(const std::string& server, const shared::TimePoint& gameStartTime, const std::string& map, const std::string& mode);
    std::string finishCsgoMatch(const std::string& viewUuid, const std::string& localSteamId, const shared::TimePoint& gameStopTime, const game_event_watcher::CsgoMatchState& state, const std::optional<std::string>& demoUrl, const std::optional<shared::TimePoint>& demoTimestamp);
    void associateCsgoDemo(const std::string& viewUuid, const std::string& demoUrl, const shared::TimePoint& demoTimestamp);

    // VOD
    service::uploader::UploadDestination createVodDestinationUri(const std::string& videoUuid, const std::string& containerFormat) const;
    service::uploader::UploadDestination getObjectPartUploadUri(const std::string& objectUuid, const std::string& bucket, const std::string& session, service::uploader::UploadPurpose uploadPurpose, int64_t part, bool needsAccel) const;
    void associateVod(const shared::squadov::VodAssociation& association, const shared::squadov::VodMetadata& metadata, const std::string& sessionUri, const std::vector<std::string>& parts) const;
    void deleteVod(const std::string& videoUuid) const;
    shared::squadov::VodAssociation getVod(const std::string& videoUuid) const;
    std::string getVodUri(const std::string& videoUuid, const std::string& format) const;
    std::string getVodMd5Checksum(const std::string& videoUuid) const;

    void createStagedClip(const std::string& videoUuid, int64_t start, int64_t end) const;

    // Speed Check
    service::uploader::UploadDestination SquadovApi::getSpeedCheckUri(const std::string& speedCheckUuid) const;
    void postSpeedCheck(const double speedMbps, const std::string& speedCheckUuid) const;

    // AWS
    AwsCognitoCredentials getAwsCredentials() const;

    // Combat Log
    std::string getCombatLogApiConfiguration() const;
    void createCombatLog(const std::string& partitionId, const shared::TimePoint& startTm, const nlohmann::json& clState) const;

    std::chrono::milliseconds generateRandomBackoff(int64_t base, size_t tryCount) const;
private:
    int64_t _sessionUpdateCbCounter = 0;
    std::unordered_map<int64_t, SessionIdUpdateCallback> _sessionUpdateCallback;
    std::mutex _sessionUpdateMutex;

    std::unique_ptr<shared::http::HttpClient> _webClient;

    // I'm not a big fan of leaving this here...
    shared::squadov::SquadOVSessionStorage _session;
    mutable std::shared_mutex _sessionMutex;

    shared::squadov::FeatureFlags _features;
    mutable std::shared_mutex _featureMutex;

    // Various caches of things we want to keep track of.
    mutable std::mutex _riotAccountOwnershipMutex;
    mutable std::unordered_set<std::string> _verifiedRiotAccounts;

    mutable std::mutex _randomMutex;
    mutable std::mt19937 _rdGen;
    std::uniform_real_distribution<> _rdDist;
    
};

using SquadovApiPtr = std::unique_ptr<SquadovApi>;

SquadovApi* getGlobalApi();

}