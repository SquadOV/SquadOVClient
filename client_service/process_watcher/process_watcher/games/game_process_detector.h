#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"

#include <memory>
#include <optional>
#include <vector>

namespace process_watcher::games {

class GameProcessDetector {
public:
    explicit GameProcessDetector(const std::string& exe);
    virtual ~GameProcessDetector() {}
    std::optional<process::Process> checkIsRunning(const process::ProcessRunningState& processes) const;
    
private:
    const std::string _exeName;
};

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game);

}