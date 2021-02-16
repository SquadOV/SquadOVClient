#pragma once

#include "process_watcher/games/game_process_detector.h"

namespace process_watcher::games {

class HearthstoneProcessDetector: public GameProcessDetector {
public:
    HearthstoneProcessDetector();
};

}