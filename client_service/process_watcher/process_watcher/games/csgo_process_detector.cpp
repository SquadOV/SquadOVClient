#include "process_watcher/games/csgo_process_detector.h"

namespace {
#ifdef _WIN32
const std::string csgoProcessName("csgo.exe");
#else
#endif

}

namespace process_watcher::games {

CsgoProcessDetector::CsgoProcessDetector():
    GameProcessDetector(csgoProcessName) {

}

}