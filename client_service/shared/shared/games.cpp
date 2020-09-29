#include "shared/games.h"

namespace shared {

std::string gameToString(EGame game) {
    switch (game) {
    case EGame::Valorant:
        return "Valorant";
    case EGame::Aimlab:
        return "Aim Lab";
    }
    return "";
}

}