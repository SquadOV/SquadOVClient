#include "process_watcher/watcher.h"
#include "process_watcher/games/game_process_detector.h"
#include "process_watcher/process/process.h"

#include <iostream>
#include <chrono>
#include <vector>

namespace process_watcher {

void ProcessWatcher::beginWatchingGame(shared::EGame game, ProcessWatchHandlerPtr&& handler) {
    std::cout << "Starting to Watch for: " << shared::gameToString(game) << std::endl;

    if (_gameToWatcher.find(game) != _gameToWatcher.end()) {
        throw std::runtime_error("Can only have 1 watcher per game.");
        return;
    }

    _gameToWatcher[game] = std::move(handler);
}

void ProcessWatcher::start() const {
    std::thread watcher([this](){
        using namespace std::chrono_literals;

        std::unordered_map<shared::EGame, bool> allLastCheckState;
        while (true) {
            // Get a sorted list of running processes from the OS.
            std::vector<process::Process> processes;
            const bool success = process::listRunningProcesses(processes);

            if (!success) {
                // If this fails something probably went very wrong.
                throw std::runtime_error("Failed to list running processes.");
            }

            for (const auto& kvp : _gameToWatcher) {
                const auto detector = games::createDetectorForGame(kvp.first);

                // Check to see if the game is running (OS-dependent).
                const bool lastCheckedState = allLastCheckState[kvp.first];
                const bool isRunning = detector->checkIsRunning(processes);
                
                // Check to see if the running state is the different from the last checked state
                // to fire off the appropriate event.
                if (isRunning != lastCheckedState) {
                    if (isRunning) {
                        kvp.second->onProcessStarts();
                    } else {
                        kvp.second->onProcessStops();
                    }
                    allLastCheckState[kvp.first] = isRunning;
                }
            }

            // Checking every 500ms should be sufficient to get everything running before a user gets into a game. 
            std::this_thread::sleep_for(500ms); 
        }
    });

    watcher.join();
}

}