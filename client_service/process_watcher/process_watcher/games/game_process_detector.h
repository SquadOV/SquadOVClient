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
    explicit GameProcessDetector(const std::vector<std::string>& exes);

    virtual ~GameProcessDetector() {}
    std::optional<process::Process> checkIsRunning(const process::ProcessRunningState& processes) const;
    
private:
    const std::vector<std::string> _exeNames;
};

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game);

}