#pragma once

#include <string>

namespace shared::valorant {

enum class EValorantRoundState {
    Buy,
    Play,
    Unknown
};

// EValorantMap
enum class EValorantMap {
    Ascent,
    Bind,
    Haven,
    Split,
    Icebox,
    Breeze,
    MainMenu,
    CharacterSelect,
    Init,
    Unknown
};

EValorantMap codenameToValorantMap(const std::string& codename);
EValorantMap mapIdToValorantMap(const std::string& id);
std::string mapToName(EValorantMap map);
bool isGameMap(EValorantMap map);

}