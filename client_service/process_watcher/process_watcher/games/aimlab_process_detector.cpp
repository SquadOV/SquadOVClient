#include "process_watcher/games/aimlab_process_detector.h"

namespace {
#ifdef _WIN32
const std::string aimlabProcessName("AimLab_tb.exe");
#else
#endif

}

namespace process_watcher::games {

bool AimlabProcessDetector::checkIsRunning(const std::vector<process::Process>& processes, size_t* outIndex) const {
    const process::Process cmpProcess(aimlabProcessName, 0);
    return checkProcessIsRunning(cmpProcess, processes, outIndex);
}

}