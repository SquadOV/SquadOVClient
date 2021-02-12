#include "process_watcher/process/process.h"
#include "process_watcher/games/game_process_detector.h"
#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/pe_mapper.h"
#include "process_watcher/memory/mono_memory_mapper.h"
#include "process_watcher/memory/games/hearthstone/hearthstone_memory_mapper.h"
#include "shared/hearthstone/hearthstone_ratings.h"
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <vector>

namespace po = boost::program_options;
using namespace process_watcher;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    desc.add_options()
        ("exe", po::value<std::string>()->required(), "Process to hook to.")
        ("module", po::value<std::string>()->required(), "PE module to dump.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    std::vector<process::Process> processes;
    const bool success = process::listRunningProcesses(processes);
    if (!success) {
        std::cerr << "Failed to list running processes." << std::endl;
        return 1;
    }

    const auto exeName = vm["exe"].as<std::string>();
    const auto module  = vm["module"].as<std::string>();
    size_t idx = 0;
    games::GameProcessDetector detector(exeName);
    const bool detected = detector.checkIsRunning(processes, &idx);
    if (!detected) {
        std::cerr << "Failed to detect process: " << exeName << std::endl;
        return 1;
    }

    const auto& process = processes[idx];
    auto memMapper = std::make_shared<memory::ModuleMemoryMapper>(process, module);
    memory::PEMapper peMapper(memMapper);
    auto monoWrapper = std::make_unique<memory::MonoMemoryMapper>(memMapper, peMapper); 
    memory::games::hearthstone::HearthstoneMemoryMapper hearthstoneMapper(std::move(monoWrapper));
    const auto deck = hearthstoneMapper.getCurrentDeck();

    std::cout << "#######################################################" << std::endl
              << "                      BNET PLAYER                      " << std::endl
              << "#######################################################" << std::endl;
    
    const auto localPlayer = hearthstoneMapper.getLocalBNetPlayer();
    if (localPlayer) {
        std::cout << "HI: " << localPlayer->hi() << "\tLO: " << localPlayer->lo() << std::endl;
    } else {
        std::cout << "No Player Found" << std::endl;
    }

    std::cout << "#######################################################" << std::endl
              << "                       SELECTED DECK                   " << std::endl
              << "#######################################################" << std::endl;

    if (deck) {
        std::cout << "Selected Deck: " << deck->toJson().dump(4) << std::endl;
    } else {
        std::cout << "Unknown Deck Selected" << std::endl;
    }

    std::cout << "#######################################################" << std::endl
              << "                     CURRENT PLAYERS                   " << std::endl
              << "#######################################################" << std::endl;
    for (const auto& kvp : hearthstoneMapper.getCurrentPlayers()) {
        std::cout << "------------------ Player " << kvp.first << "------------------" << std::endl
                  << kvp.second->toJson().dump(4) << std::endl;
    }

    std::cout << "#######################################################" << std::endl
              << "                       NET CACHE                       " << std::endl
              << "#######################################################" << std::endl;
    std::cout << "Battlegrounds Rating: " << hearthstoneMapper.getBattlegroundsRating().value_or(-1) << std::endl;
    std::cout << "Duels Casual Rating: " << hearthstoneMapper.getDuelsCasualRating().value_or(-1) << std::endl;
    std::cout << "Duels Heroic Rating: " << hearthstoneMapper.getDuelsHeroicRating().value_or(-1) << std::endl;

    shared::hearthstone::HearthstoneRatings ratings;
    ratings.battlegroundsRating = hearthstoneMapper.getBattlegroundsRating();
    ratings.duelsCasualRating = hearthstoneMapper.getDuelsCasualRating();
    ratings.duelsHeroicRating = hearthstoneMapper.getDuelsHeroicRating();
    std::cout << ratings.toJson().dump(4) << std::endl;

    return 0;
}