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
    case EGame::LeagueOfLegends:
        return "League of Legends";
    case EGame::TFT:
        return "Teamfight Tactics";
    case EGame::CSGO:
        return "Counter Strike: Global Offensive";
    case EGame::WowVanilla:
        return "World of Warcraft Classic";
    case EGame::WowTbc:
        return "Burning Crusade Classic";
    case EGame::Ff14:
        return "Final Fantasy 14";
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
    case EGame::LeagueOfLegends:
        return "lol";
    case EGame::TFT:
        return "tft";
    case EGame::CSGO:
        return "csgo";
    case EGame::WowVanilla:
        return "wowc";
    case EGame::WowTbc:
        return "tbc";
    case EGame::Ff14:
        return "ff14";
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

bool isWowClassic(EGame game) {
    switch (game) {
        case EGame::WowVanilla:
        case EGame::WowTbc:
            return true;
        default:
            return false;
    }
}

EWowRelease gameToWowRelease(EGame game) {
    switch (game) {
        case EGame::WoW:
            return EWowRelease::Retail;
        case EGame::WowVanilla:
            return EWowRelease::Vanilla;
        case EGame::WowTbc:
            return EWowRelease::Tbc;
        default:
            return EWowRelease::Unknown;
    }
}

}