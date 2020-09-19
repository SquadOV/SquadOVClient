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
    bool isValorantAccountSynced(const std::string& puuid) const;
    void markValorantAccountSync(const std::string& puuid) const;

private:
    sqlite3* _db = nullptr;
};
using DatabaseApiPtr = std::unique_ptr<DatabaseApi>;
    
}