#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <fstream>
#include <memory>
#include <optional>
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
        bool immediatelyGoToEnd = false,
        bool loop = true,
        std::optional<typename std::ifstream::pos_type> initialPosition = std::nullopt 
    );
    ~LogWatcher();

    void disableBatching() { _batchingEnabled = false; }

    const std::filesystem::path& path() const { return _path; }
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
    bool _loop = true;
    std::optional<std::ifstream::pos_type> _initialPosition = std::nullopt;
};

using LogWatcherPtr = std::unique_ptr<LogWatcher>;

}