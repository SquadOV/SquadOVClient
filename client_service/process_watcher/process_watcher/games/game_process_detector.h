#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"

#include <memory>
#include <vector>

namespace process_watcher::games {

class GameProcessDetector {
public:
    explicit GameProcessDetector(const std::string& exe);
    virtual ~GameProcessDetector() {}
    bool checkIsRunning(const std::vector<process::Process>& processes, size_t* outIndex = nullptr) const;

protected:
    bool checkProcessIsRunning(const process::Process& ref, const std::vector<process::Process>& processes, size_t* outIndex = nullptr) const;

private:
    const std::string _exeName;
};

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game);

}