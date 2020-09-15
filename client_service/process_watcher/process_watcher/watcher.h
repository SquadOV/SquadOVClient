#pragma once

#include "shared/games.h"
#include "process_watcher/handler.h"

#include <unordered_map>
#include <thread>

namespace process_watcher {

class ProcessWatcher {
public:
    void beginWatchingGame(shared::EGame game, ProcessWatchHandlerPtr&& handler);
    void start() const;

private:
    std::unordered_map<shared::EGame, ProcessWatchHandlerPtr> _gameToWatcher;
};

}