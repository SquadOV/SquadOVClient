#pragma once

#include "shared/riot/riot.h"
#include "shared/aimlab/aimlab.h"
#include "valorant/valorant_match.h"
#include "valorant/valorant_match_details.h"

#include <memory>
#include <sqlite3.h>
#include <string>
#include <vector>

namespace service::valorant {

class ValorantMatch;

}

namespace service::database {

class DatabaseApi {
public:
    explicit DatabaseApi(const std::string& dbFname);
    ~DatabaseApi();

    // Valorant related functions.
    void storeValorantAccount(const shared::riot::RiotUser& user) const;
    void storeValorantMatch(const service::valorant::ValorantMatch* match) const;

    service::valorant::ValorantMatchPtr getValorantMatch(const std::string& matchId) const;
    std::vector<service::valorant::ValorantMatchPlayerPtr> getValorantMatchPlayers(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const;
    std::vector<service::valorant::ValorantMatchTeamPtr> getValorantMatchTeams(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const;
    std::vector<service::valorant::ValorantMatchRoundPtr> getValorantMatchRounds(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const;
    std::vector<service::valorant::ValorantMatchLoadoutPtr> getValorantMatchRoundLoadouts(const std::string& matchId, int roundNum, const service::valorant::ValorantMatchDetails& details) const;
    std::unordered_map<std::string, int> getValorantMatchRoundStats(const std::string& matchId, int roundNum) const;

    std::vector<service::valorant::ValorantMatchKillPtr> getValorantMatchKills(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const;
    std::vector<service::valorant::ValorantMatchDamagePtr> getValorantMatchDamage(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const;

    size_t totalValorantMatchesForPuuid(const std::string& puuid, bool withApiData) const;
    bool isValorantMatchStored(const std::string& matchId) const;
    void associateValorantMatchToVideoFile(const std::string& matchId, const std::string& fname) const;
    std::string getVodFilenameForValorantMatch(const std::string& matchId) const;
    bool isValorantVideoAssociatedWithMatch(const std::string& fname) const;
    std::vector<std::string> getMatchIdsForPlayer(const std::string& puuid) const;

    // Aim Lab related fucntions.
    void storeAimlabTask(const shared::aimlab::TaskData& task, const std::string& vodPath) const;
    bool isAimlabVideoAssociatedWithTask(const std::string& fname) const;
    std::vector<int> allStoredAimlabTaskIds() const;

private:
    sqlite3* _db = nullptr;
};
using DatabaseApiPtr = std::unique_ptr<DatabaseApi>;
    
}