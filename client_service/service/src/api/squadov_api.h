#pragma once

#include "http/http_client.h"
#include "shared/riot/riot.h"
#include "shared/squadov/session.h"
#include "shared/squadov/vod.h"
#include "shared/aimlab/aimlab.h"
#include "shared/hearthstone/hearthstone_ratings.h"

#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "game_event_watcher/wow/wow_log_watcher.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/player_mapper.h"

#include <functional>
#include <memory>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <vector>

namespace service::api {

using SessionIdUpdateCallback = std::function<void(const std::string&)>;

// A mix between our local API (for however much longer we need to keep it around for)
// and our we bapi.
class SquadovApi {
public:
    SquadovApi();

    void setSessionIdUpdateCallback(const SessionIdUpdateCallback& cb) { _sessionUpdateCallback = cb; }
    void setSessionId(const std::string& key);

    int64_t getSessionUserId() const;
    std::string getSessionUserUuid() const;

    // User
    shared::squadov::SquadOVUser getCurrentUser() const;

    // Valorant
    shared::riot::RiotRsoToken refreshValorantRsoToken(const std::string& puuid) const;
    shared::riot::RiotUser getRiotUserFromPuuid(const std::string& puuid) const;
    shared::riot::RiotUser updateRiotUsername(const std::string& puuid, const std::string& username, const std::string& tagline) const;
    shared::riot::RiotUser updateRiotUsernameApi(const std::string& puuid, const std::string& username, const std::string& tagline) const;

    std::string uploadValorantMatch(const std::string& matchId, const nlohmann::json& rawData, const nlohmann::json& playerData) const;
    std::vector<std::string> obtainMissingValorantMatches(const std::vector<std::string>& ids) const;

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
    std::string obtainNewWoWCombatLogUuid(const game_event_watcher::WoWCombatLogState& log) const;
    void uploadWoWCombatLogLine(const std::string& combatLogUuid, const game_event_watcher::RawWoWCombatLog& log) const;
    std::string createWoWChallengeMatch(const shared::TimePoint& timestamp, const std::string& combatLogUuid, const game_event_watcher::WoWChallengeModeStart& encounter, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants);
    void finishWoWChallengeMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWChallengeModeEnd& encounter);
    std::string createWoWEncounterMatch(const shared::TimePoint& timestamp, const std::string& combatLogUuid, const game_event_watcher::WoWEncounterStart& encounter, const std::vector<game_event_watcher::WoWCombatantInfo>& combatants);
    void finishWoWEncounterMatch(const std::string& matchUuid, const shared::TimePoint& timestamp, const game_event_watcher::WoWEncounterEnd& encounter);

    // VOD
    std::string createVodDestinationUri(const std::string& videoUuid) const;
    void associateVod(const shared::squadov::VodAssociation& association, const shared::squadov::VodMetadata& metadata, const std::string& sessionUri) const;
    void deleteVod(const std::string& videoUuid) const;

private:
    SessionIdUpdateCallback _sessionUpdateCallback;

    std::unique_ptr<http::HttpClient> _localClient;
    std::unique_ptr<http::HttpClient> _webClient;

    // I'm not a big fan of leaving this here...
    shared::squadov::SquadOVSessionStorage _session;
    mutable std::shared_mutex _sessionMutex;
};

using SquadovApiPtr = std::unique_ptr<SquadovApi>;

SquadovApi* getGlobalApi();

}