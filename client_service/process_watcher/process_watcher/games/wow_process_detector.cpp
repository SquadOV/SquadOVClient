#include "process_watcher/games/wow_process_detector.h"

namespace {
#ifdef _WIN32
const std::string wowProcessName("Wow.exe");
#else
#endif

}

namespace process_watcher::games {

WoWProcessDetector::WoWProcessDetector():
    GameProcessDetector(wowProcessName) {

}

}