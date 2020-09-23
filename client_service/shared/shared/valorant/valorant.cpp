#include "shared/valorant/valorant.h"
#include "shared/strings/strings.h"

namespace shared::valorant {

EValorantMap mapIdToValorantMap(const std::string& id) {
    if (id == "/Game/Maps/Ascent/Ascent") {
        return EValorantMap::Ascent;
    } else if (id == "/Game/Maps/Triad/Triad") {
        return EValorantMap::Haven;
    } else if (id == "/Game/Maps/Duality/Duality") {
        return EValorantMap::Bind;
    } else if (id == "/Game/Maps/Bonsai/Bonsai") {
        return EValorantMap::Split;
    }
    return EValorantMap::Unknown;
}

EValorantMap codenameToValorantMap(const std::string& codename) {
    if (codename == "Ascent") {
        return EValorantMap::Ascent;
    } else if (codename == "Triad") {
        return EValorantMap::Haven;
    } else if (codename == "Duality") {
        return EValorantMap::Bind;
    } else if (codename == "Bonsai") {
        return EValorantMap::Split;
    } else if (codename == "MainMenuV2") {
        return EValorantMap::MainMenu;
    } else if (codename == "Init"){ 
        return EValorantMap::Init;
    } else if (codename == "CharacterSelectPersistentLevel") {
        return EValorantMap::CharacterSelect;
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
        case EValorantMap::Init:
            return "Init";
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