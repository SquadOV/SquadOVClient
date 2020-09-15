#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"

#include <memory>
#include <vector>

namespace process_watcher::games {

class GameProcessDetector {
public:
    virtual bool checkIsRunning(const std::vector<process::Process>& processes) const = 0;
};

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game);

}