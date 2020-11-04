#include "process_watcher/games/game_process_detector.h"

#include "process_watcher/games/valorant_process_detector.h"
#include "process_watcher/games/aimlab_process_detector.h"
#include "process_watcher/games/hearthstone_process_detector.h"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace process_watcher::games {

GameProcessDetector::GameProcessDetector(const std::string& exe):
    _exeName(exe) {
    
}

bool GameProcessDetector::checkIsRunning(const std::vector<process::Process>& processes, size_t* outIndex) const {
    const process::Process cmpProcess(_exeName, 0);
    return checkProcessIsRunning(cmpProcess, processes, outIndex);
}

bool GameProcessDetector::checkProcessIsRunning(const process::Process& ref, const std::vector<process::Process>& processes, size_t* outIndex) const {
    const auto fnCmp = [](const process::Process& a, const process::Process& b){
        return a.name() < b.name();
    };

    if (!outIndex) {
        const bool found = std::binary_search(processes.begin(), processes.end(), ref, fnCmp);
        return found;
    } else {
        const auto it = std::lower_bound(processes.begin(), processes.end(), ref, fnCmp);
        if (it == processes.end()) {
            return false;
        }
        const size_t idx = std::distance(processes.begin(), it);
        if (processes[idx].name() != ref.name()) {
            return false;
        }

        *outIndex =idx;
        return true;
    }
}

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game) {
    switch (game) {
        case shared::EGame::Valorant:
            return std::make_unique<ValorantProcessDetector>();
        case shared::EGame::Aimlab:
            return std::make_unique<AimlabProcessDetector>();
        case shared::EGame::Hearthstone:
            return std::make_unique<HearthstoneProcessDetector>();
    }
    return nullptr;
}


}