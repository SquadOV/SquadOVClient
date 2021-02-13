#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace shared {

enum class EGame {
    Valorant,
    Aimlab,
    Hearthstone,
    WoW,
    LeagueOfLegends,
    TFT,
};

using EGameSet = std::unordered_set<EGame>;

std::string gameToString(EGame game);
std::string gameToIconString(EGame game);
std::vector<EGame> gameSetToVector(const EGameSet& set);
nlohmann::json gameVectorToJsonArray(const std::vector<EGame>& games);

}