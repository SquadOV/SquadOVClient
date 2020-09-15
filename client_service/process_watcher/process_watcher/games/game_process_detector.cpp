#include "process_watcher/games/game_process_detector.h"

#include "process_watcher/games/valorant_process_detector.h"

namespace process_watcher::games {

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game) {
    switch (game) {
    case shared::EGame::Valorant:
        return std::make_unique<ValorantProcessDetector>();        
    }
    return nullptr;
}

}