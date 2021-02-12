#include "process_watcher/games/league_process_detector.h"

namespace {
#ifdef _WIN32
const std::string leagueProcessName("League of Legends.exe");
#else
#endif

}

namespace process_watcher::games {

LeagueProcessDetector::LeagueProcessDetector():
    GameProcessDetector(leagueProcessName) {

}

}