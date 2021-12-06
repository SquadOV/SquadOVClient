#include "process_watcher/games/game_process_detector.h"

#include "process_watcher/games/valorant_process_detector.h"
#include "process_watcher/games/aimlab_process_detector.h"
#include "process_watcher/games/hearthstone_process_detector.h"
#include "process_watcher/games/wow_process_detector.h"
#include "process_watcher/games/league_process_detector.h"
#include "process_watcher/games/csgo_process_detector.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace process_watcher::games {

GameProcessDetector::GameProcessDetector(const std::string& exe):
    _exeNames({exe})
{

}

GameProcessDetector::GameProcessDetector(const std::vector<std::string>& exes):
    _exeNames(exes)
{
    
}

std::optional<process::Process> GameProcessDetector::checkIsRunning(const process::ProcessRunningState& processes) const {
    for (const auto& exe : _exeNames) {
        const process::Process cmpProcess(std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>(), exe, 0);
        const auto p = processes.getProcesssRunningByName(cmpProcess.name(), true);
        if (!p.empty()) {
            return p[0];
        }
    }
    return std::nullopt;
}

std::unique_ptr<GameProcessDetector> createDetectorForGame(shared::EGame game) {
    switch (game) {
        case shared::EGame::Valorant:
            return std::make_unique<ValorantProcessDetector>();
        case shared::EGame::Aimlab:
            return std::make_unique<AimlabProcessDetector>();
        case shared::EGame::Hearthstone:
            return std::make_unique<HearthstoneProcessDetector>();
        case shared::EGame::WoW:
        case shared::EGame::WowVanilla:
        case shared::EGame::WowTbc:
            return std::make_unique<WoWProcessDetector>();
        case shared::EGame::LeagueOfLegends:
            return std::make_unique<LeagueProcessDetector>();
        case shared::EGame::CSGO:
            return std::make_unique<CsgoProcessDetector>();
    }
    return nullptr;
}


}