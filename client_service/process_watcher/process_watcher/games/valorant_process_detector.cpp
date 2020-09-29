#include "process_watcher/games/valorant_process_detector.h"

namespace {
#ifdef _WIN32
const std::string valorantProcessName("VALORANT-Win64-Shipping.exe");
#else
#endif

}

namespace process_watcher::games {

bool ValorantProcessDetector::checkIsRunning(const std::vector<process::Process>& processes, size_t* outIndex) const {
    const process::Process cmpProcess(valorantProcessName, 0);
    return checkProcessIsRunning(cmpProcess, processes, outIndex);
}

}