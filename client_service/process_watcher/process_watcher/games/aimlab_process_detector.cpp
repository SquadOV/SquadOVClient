#include "process_watcher/games/aimlab_process_detector.h"

namespace {
#ifdef _WIN32
const std::string aimlabProcessName("AimLab.exe");
const std::string aimlabProcessName2("AimLab_tb.exe");
#else
#endif

}

namespace process_watcher::games {

AimlabProcessDetector::AimlabProcessDetector():
    GameProcessDetector({aimlabProcessName, aimlabProcessName2}) {

}

}