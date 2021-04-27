#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"

namespace game_event_watcher {

enum class ECsgoLogEvents {
    Connect,
    Disconnect,
};

using CsgoLogWatcherPtr = std::unique_ptr<class CsgoLogWatcher>;
class CsgoLogWatcher: public BaseLogWatcher {
public:
    static void enableCsgoLogging();

    explicit CsgoLogWatcher(const shared::TimePoint& timeThreshold);

private:
    void onGameLogChange(const LogLinesDelta& lines);
    
    LogWatcherPtr _gameLogWatcher;
};

}