#pragma once

#include "http/http_client.h"

#include <memory>
#include <string>

namespace service::api {

struct LeagueGameStats {
    std::string gameMode;
    double gameTime;
    std::string mapName;
    int mapNumber;
};

class LeagueIngameApi {
public:
    LeagueIngameApi();

    LeagueGameStats getGameStats();
    std::string getActivePlayerName();

private:
    std::unique_ptr<http::HttpClient> _client;
};
using LeagueIngameApiPtr = std::unique_ptr<LeagueIngameApi>;

}