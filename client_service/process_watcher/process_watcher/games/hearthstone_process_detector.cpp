#include "process_watcher/games/hearthstone_process_detector.h"

namespace {
#ifdef _WIN32
const std::string HearthstoneProcessName("Hearthstone.exe");
#else
#endif

}

namespace process_watcher::games {

HearthstoneProcessDetector::HearthstoneProcessDetector():
    GameProcessDetector(HearthstoneProcessName) {
}

}