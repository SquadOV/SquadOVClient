#include "shared/valorant/valorant.h"

namespace shared::valorant {

EValorantMap codenameToValorantMap(const std::string& codename) {
    if (codename == "Ascent") {
            return EValorantMap::Ascent;
    } else if (codename == "Triad") {
            return EValorantMap::Haven;
    } else if (codename == "Duality") {
            return EValorantMap::Bind;
    } else if (codename == "Bonsai") {
            return EValorantMap::Split;
    }
    return EValorantMap::Unknown;
}

std::string mapToName(EValorantMap map) {
    switch (map) {
        case EValorantMap::Ascent:
            return "Ascent";
        case EValorantMap::Bind:
            return "Bind";
        case EValorantMap::Haven:
            return "Haven";
        case EValorantMap::Split:
            return "Split";
        case EValorantMap::MainMenu:
            return "Main Menu";
        case EValorantMap::CharacterSelect:
            return "Character Select";
    }
    return "Unknown";
}

bool isGameMap(EValorantMap map) {
    switch (map) {
        case EValorantMap::Ascent:
        case EValorantMap::Bind:
        case EValorantMap::Haven:
        case EValorantMap::Split:
            return true;
    }    
    return false;
}

}