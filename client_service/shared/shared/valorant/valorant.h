#pragma once

#include <string>

namespace shared::valorant {

enum class EValorantMap {
    Ascent,
    Bind,
    Haven,
    Split,
    MainMenu,
    CharacterSelect,
    Unknown
};

enum class EValorantRoundState {
    Buy,
    Play
};

EValorantMap codenameToValorantMap(const std::string& codename);
std::string mapToName(EValorantMap map);
bool isGameMap(EValorantMap map);

}