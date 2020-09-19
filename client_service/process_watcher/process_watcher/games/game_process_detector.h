#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"

#include <memory>
#include <vector>

namespace process_watcher::games {

class GameProcessDetector {
public:
    virtual ~GameProcessDetector() {}
    virtual bool checkIsRunning(const std::vector<process::Process>& processes, size_t* outIndex = nullptr) const = 0;
};

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game);

}