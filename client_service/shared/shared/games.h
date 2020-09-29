#pragma once

#include <string>

namespace shared {

enum class EGame {
    Valorant,
    Aimlab
};

std::string gameToString(EGame game);

}