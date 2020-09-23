#pragma once

#include "shared/riot/riot.h"

#include <memory>
#include <sqlite3.h>
#include <string>

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
    size_t totalValorantMatchesForPuuid(const std::string& puuid) const;
    bool isValorantMatchStored(const std::string& matchId) const;
    void associateValorantMatchToVideoFile(const std::string& matchId, const std::string& fname) const;
    bool isValorantVideoAssociatedWithMatch(const std::string& fname) const;

private:
    sqlite3* _db = nullptr;
};
using DatabaseApiPtr = std::unique_ptr<DatabaseApi>;
    
}