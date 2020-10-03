#pragma once

#include "http/http_client.h"
#include "shared/riot/riot.h"

#include <memory>

namespace service::api {

class SquadovApi {
public:
    SquadovApi();

    // Valorant
    shared::riot::RiotRsoToken refreshValorantRsoToken(const std::string& puuid) const;
    shared::riot::RiotUser getRiotUserFromPuuid(const std::string& puuid) const;

private:
    std::unique_ptr<http::HttpClient> _client;
};

using SquadovApiPtr = std::unique_ptr<SquadovApi>;

SquadovApi* getGlobalApi();

}