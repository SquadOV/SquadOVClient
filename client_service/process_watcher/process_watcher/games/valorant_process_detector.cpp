#include "process_watcher/games/valorant_process_detector.h"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace {
#ifdef _WIN32
const std::string valorantProcessName("VALORANT-Win64-Shipping.exe");
#else
#endif

}

namespace process_watcher::games {

bool ValorantProcessDetector::checkIsRunning(const std::vector<process::Process>& processes, size_t* outIndex) const {
    const process::Process cmpProcess(valorantProcessName, 0);
    const auto fnCmp = [](const process::Process& a, const process::Process& b){
        return a.name() < b.name();
    };

    if (!outIndex) {
        const bool found = std::binary_search(processes.begin(), processes.end(), cmpProcess, fnCmp);
        return found;
    } else {
        const auto it = std::lower_bound(processes.begin(), processes.end(), cmpProcess, fnCmp);
        if (it == processes.end()) {
            return false;
        }
        const size_t idx = std::distance(processes.begin(), it);
        if (processes[idx].name() != cmpProcess.name()) {
            return false;
        }

        *outIndex =idx;
        return true;
    }
}

}