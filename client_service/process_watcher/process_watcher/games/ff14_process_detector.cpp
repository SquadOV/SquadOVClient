#include "process_watcher/games/ff14_process_detector.h"

namespace {
#ifdef _WIN32
const std::string ff14ProcessName("ffxiv.exe");
const std::string ff14Dx11ProcesName("ffxiv_dx11.exe");
#else
#endif

}

namespace process_watcher::games {

Ff14ProcessDetector::Ff14ProcessDetector():
    GameProcessDetector({ff14ProcessName, ff14Dx11ProcesName}) {

}

}