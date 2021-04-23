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

                size_t processIndex = 0;
                const bool isRunning = detector->checkIsRunning(processes, &processIndex);
                
                const auto& watchedProcess = kvp.second->currentProcess();
                // Check to see if the running state is the different from the last checked state
                // to fire off the appropriate event. 
                if (isRunning) {
                    // If we're running, we want to see if the PID changed which would indicate a state change.
                    // If we previously didn't have a PID for the EXE, then that means this process just started (fresh).
                    // If we previously did have a PID for the EXE, then that means the user restarted the app really quickly
                    // and we need to restart the game's watcher as well.
                    const auto& newProcess = processes[processIndex];
                    if (newProcess.pid() != watchedProcess.pid()) {
                        if (!watchedProcess.empty()) {
                            kvp.second->onProcessStops();
                        }

                        LOG_INFO("Detected Process Start: " << newProcess.path() << " - " << newProcess.pid() << std::endl);
                        kvp.second->publicOnProcessStarts(newProcess);
                    }
                } else if (!watchedProcess.empty()) {
                    // If we're no longer running then we need to stop the watcher if it's watching something.
                    kvp.second->onProcessStops();
                }
            }

            // Checking every 500ms should be sufficient to get everything running before a user gets into a game. 
            std::this_thread::sleep_for(500ms); 
        }
    });
}

}