#include "process_watcher/watcher.h"
#include "process_watcher/games/game_process_detector.h"
#include "process_watcher/process/process.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"

#include <iostream>
#include <chrono>
#include <vector>

namespace process_watcher {

void ProcessWatcher::beginWatchingGame(shared::EGame game, ProcessWatchHandlerPtr&& handler) {
    LOG_INFO("Starting to Watch for: " << shared::gameToString(game) << std::endl);
    
    std::unique_lock<std::shared_mutex> guard(_mapMutex);
    if (_gameToWatcher.find(game) != _gameToWatcher.end()) {
        THROW_ERROR("Can only have 1 watcher per game.");
        return;
    }

    _gameToWatcher[game] = std::move(handler);
}

void ProcessWatcher::start() {
    _running = true;
    _watchThread = std::thread([this](){
        using namespace std::chrono_literals;

        std::unordered_map<shared::EGame, bool> allLastCheckState;
        while (_running) {
            // Get a sorted list of running processes from the OS.
            std::vector<process::Process> processes;
            const bool success = process::listRunningProcesses(processes);

            if (!success) {
                // If this fails something probably went very wrong.
                THROW_ERROR("Failed to list running processes.");
            }

            std::shared_lock<std::shared_mutex> guard(_mapMutex);
            for (const auto& kvp : _gameToWatcher) {
                const auto detector = games::createDetectorForGame(kvp.first);

                // Check to see if the game is running (OS-dependent).
                const bool lastCheckedState = allLastCheckState[kvp.first];
                size_t processIndex = 0;
                const bool isRunning = detector->checkIsRunning(processes, &processIndex);
                
                // Check to see if the running state is the different from the last checked state
                // to fire off the appropriate event.
                if (isRunning != lastCheckedState) {
                    if (isRunning) {
                        kvp.second->onProcessStarts(processes[processIndex]);
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
    _watchThread.join();
}

}