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
    } else if (id == "/Game/Maps/Port/Port") {
        return EValorantMap::Icebox;
    } else if (id == "/Game/Maps/Foxtrot/Foxtrot") {
        return EValorantMap::Breeze;
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
    } else if (codename == "Port") {
        return EValorantMap::Icebox;
    } else if (codename == "Foxtrot") {
        return EValorantMap::Breeze;
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
        case EValorantMap::Icebox:
            return "Icebox";
        case EValorantMap::Breeze:
            return "Breeze";
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
        case EValorantMap::Icebox:
        case EValorantMap::Breeze:
            return true;
    }    
    return false;
}

EValorantGameMode gameIdToValorantMode(const std::string& id) {
    if (id.rfind("/Game/GameModes/Bomb/BombGameMode", 0) != std::string::npos) {
        return EValorantGameMode::Standard;
    } else if (id.rfind("/Game/GameModes/QuickBomb/QuickBombGameMode", 0) != std::string::npos) {
        return EValorantGameMode::SpikeRush;
    } else if (id.rfind("/Game/GameModes/Deathmatch/DeathmatchGameMode", 0) != std::string::npos) {
        return EValorantGameMode::Deathmatch;
    }
    return EValorantGameMode::Unknown;
}

std::string gameModeToName(EValorantGameMode mode) {
    switch (mode) {
        case EValorantGameMode::Standard:
            return "Standard";
        case EValorantGameMode::SpikeRush:
            return "Spike Rush";
        case EValorantGameMode::Deathmatch:
            return "Deathmatch";
    }
    return "Unknown";
}

}