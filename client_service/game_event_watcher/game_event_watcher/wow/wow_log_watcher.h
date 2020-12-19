#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "shared/time.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace game_event_watcher {

enum class EWoWLogEvents {
};

class WoWLogWatcher : public BaseLogWatcher {
public:
    WoWLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold);
    void loadFromExecutable(const std::filesystem::path& exePath);
private:
    void onCombatLogChange(const LogLinesDelta& lines);

    std::filesystem::path _logPath;
    LogWatcherPtr _watcher;
};

using WoWLogWatcherPtr = std::unique_ptr<WoWLogWatcher>;

}