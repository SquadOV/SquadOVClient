#include "process_watcher/games/act_process_detector.h"

namespace {
#ifdef _WIN32
const std::string actProcessName("Advanced Combat Tracker.exe");
#else
#endif

}

namespace process_watcher::games {

ActProcessDetector::ActProcessDetector():
    GameProcessDetector({actProcessName})
{
}

}