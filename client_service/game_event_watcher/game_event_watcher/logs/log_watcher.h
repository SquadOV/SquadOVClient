#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "shared/time.h"

namespace game_event_watcher {

using LogLinesDelta = std::vector<std::string>;
// Callback receives a vector of strings - one for each new line that was added.
using LogChangeCallback = std::function<void(const LogLinesDelta&)>;

class LogWatcher {
public:
    using DelayType = std::chrono::duration<unsigned long long, std::milli>;

    LogWatcher(
        const std::filesystem::path& path,
        const LogChangeCallback& cb,
        const shared::TimePoint& timeThreshold,
        bool waitForNewFile = false,
        bool immediatelyGoToEnd = false
    );
    ~LogWatcher();

    void disableBatching() { _batchingEnabled = false; }

    void wait();
private:
    void watchWorker();

    const std::filesystem::path _path;
    const LogChangeCallback _cb;
    const shared::TimePoint _timeThreshold;
    const bool _waitForNewFile;

    std::thread _changeThread;
    bool _isFinished = false;
    bool _batchingEnabled = true;
    bool _immediatelyGoToEnd = false;
};

using LogWatcherPtr = std::unique_ptr<LogWatcher>;

}