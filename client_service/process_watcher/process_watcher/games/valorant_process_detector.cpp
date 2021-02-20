#include "process_watcher/games/valorant_process_detector.h"

namespace {
#ifdef _WIN32
const std::string valorantProcessName("VALORANT-Win64-Shipping.exe");
#else
#endif

}

namespace process_watcher::games {

ValorantProcessDetector::ValorantProcessDetector():
    GameProcessDetector(valorantProcessName) {
}

}