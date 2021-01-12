#pragma once
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
};
using LeagueIngameApiPtr = std::unique_ptr<LeagueIngameApi>;

}