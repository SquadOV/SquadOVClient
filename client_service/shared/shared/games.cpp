#include "shared/games.h"

namespace shared {

std::string gameToString(EGame game) {
    switch (game) {
    case EGame::Valorant:
        return "Valorant";
    case EGame::Aimlab:
        return "Aim Lab";
    case EGame::Hearthstone:
        return "Hearthstone";
    case EGame::WoW:
        return "World of Warcraft";
    }
    return "";
}

std::string gameToIconString(EGame game) {
    switch (game) {
    case EGame::Valorant:
        return "valorant";
    case EGame::Aimlab:
        return "aimlab";
    case EGame::Hearthstone:
        return "hearthstone";
    case EGame::WoW:
        return "wow";
    }
    return "";
}

std::vector<EGame> gameSetToVector(const EGameSet& set) {
    return std::vector<EGame>(set.begin(), set.end());
}

nlohmann::json gameVectorToJsonArray(const std::vector<EGame>& games) {
    nlohmann::json data = nlohmann::json::array();
    for (const auto& g: games){
        data.push_back(gameToIconString(g));
    }
    return data;
}

}