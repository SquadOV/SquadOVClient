#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace game_event_watcher {

// Callback receives a vector of strings - one for each new line that was added.
using LogLinesDelta = std::vector<std::string>;
using LogChangeCallback = std::function<void(const LogLinesDelta&)>;

class LogWatcher {
public:
    using DelayType = std::chrono::duration<unsigned long long, std::milli>;

    LogWatcher(
        const std::filesystem::path& path,
        const LogChangeCallback& cb,
        const bool waitForNewFile = false
    );
    ~LogWatcher();

private:
    void watchWorker();

    const std::filesystem::path _path;
    const LogChangeCallback _cb;
    const bool _waitForNewFile;

    std::thread _changeThread;
    bool _isFinished = false;
};

using LogWatcherPtr = std::unique_ptr<LogWatcher>;

}