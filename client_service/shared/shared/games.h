#pragma once

#include <string>

namespace shared {

enum class EGame {
    Valorant,
    Aimlab,
    Hearthstone,
    WoW
};

std::string gameToString(EGame game);

}