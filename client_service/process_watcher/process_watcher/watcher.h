#pragma once

#include "shared/games.h"
#include "process_watcher/handler.h"
#include "process_watcher/games/game_process_detector.h"

#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <thread>

namespace process_watcher {

class ProcessWatcher {
public:
    void beginWatchingGame(shared::EGame game, ProcessWatchHandlerPtr&& handler);
    void start();

private:
    std::unordered_map<shared::EGame, ProcessWatchHandlerPtr> _gameToWatcher;
    std::shared_mutex _mapMutex;

    std::thread _watchThread;
    bool _running = false;
};

std::optional<process::Process> isGameRunning(shared::EGame game);
std::optional<process::Process> isProcessRunning(const games::GameProcessDetector& detector);

}