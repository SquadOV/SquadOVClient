#include "process_watcher/watcher.h"
#include "process_watcher/process/process.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"

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

        process::ProcessRunningState processState(std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>());
        while (_running) {
            // Get a sorted list of running processes from the OS.
            const bool success = processState.update();

            if (!success) {
                // If this fails something probably went very wrong.
                THROW_ERROR("Failed to list running processes.");
            }

            std::shared_lock<std::shared_mutex> guard(_mapMutex);
            for (const auto& kvp : _gameToWatcher) {
                const auto detector = games::createDetectorForGame(kvp.first);
                const auto newProcess = detector->checkIsRunning(processState);
                
                const auto& watchedProcess = kvp.second->currentProcess();
                if (newProcess) {
                    // If we're running, we want to see if the PID changed which would indicate a state change.
                    // If we previously didn't have a PID for the EXE, then that means this process just started (fresh).
                    // If we previously did have a PID for the EXE, then that means the user restarted the app really quickly
                    // and we need to restart the game's watcher as well.
                    if (newProcess->pid() != watchedProcess.pid()) {
                        if (!watchedProcess.empty()) {
                            if (!kvp.second->onProcessChange(*newProcess)) {
                                kvp.second->publicOnProcessStops();
                            }
                        }

                        LOG_INFO("Detected Process Start: " << newProcess->path() << " - " << newProcess->pid() << std::endl);
                        kvp.second->publicOnProcessStarts(*newProcess);
                    }
                } else if (!watchedProcess.empty()) {
                    // If we're no longer running then we need to stop the watcher if it's watching something.
                    kvp.second->publicOnProcessStops();
                }
            }

            // Checking every 500ms should be sufficient to get everything running before a user gets into a game. 
            std::this_thread::sleep_for(500ms); 
        }
    });
}

std::optional<process::Process> isGameRunning(shared::EGame game) {
    const auto detector = games::createDetectorForGame(game);
    if (!detector) {
        LOG_WARNING("...Failed to create detector for game: " << shared::gameToString(game) << std::endl);
        return std::nullopt;
    }
    return isProcessRunning(*detector);
}

std::optional<process::Process> isProcessRunning(const games::GameProcessDetector& detector) {
    process::ProcessRunningState processState(std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>());
    processState.update();
    return detector.checkIsRunning(processState);
}

}