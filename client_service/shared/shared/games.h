#pragma once

#include <string>

namespace shared {

enum class EGame {
    Valorant,
    Aimlab,
    Hearthstone
};

std::string gameToString(EGame game);

}