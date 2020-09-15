#include "process_watcher/games/valorant_process_detector.h"

#include <algorithm>
#include <iostream>

namespace {
#ifdef _WIN32
const std::string valorantProcessName("VALORANT.exe");
#else
#endif

}

namespace process_watcher::games {

bool ValorantProcessDetector::checkIsRunning(const std::vector<process::Process>& processes) const {
    const process::Process cmpProcess(valorantProcessName);
    const bool found = std::binary_search(processes.begin(), processes.end(), cmpProcess, [](const process::Process& a, const process::Process& b){
        return a.name() < b.name();
    });
    return found;
}

}