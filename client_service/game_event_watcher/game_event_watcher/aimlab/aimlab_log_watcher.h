#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "shared/time.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace game_event_watcher {

enum class EAimlabLogEvents {
    StartTask,
    RestartTask,
    KillTask,
    FinishTask
};

struct AimlabLogState {
    bool inTask = false;
    std::string taskName;
    std::string taskMode;
    std::string taskMap;
    std::string gameVersion;

    bool isInTask() const;
};

bool operator==(const AimlabLogState& a, const AimlabLogState& b);

class AimlabLogWatcher : public BaseLogWatcher {
public:
    AimlabLogWatcher();

private:
    void onGameLogChange(const LogLinesDelta& lines);

    std::filesystem::path _logPath;
    LogWatcherPtr _watcher;

    // Log State. The only real two states there are for Aim Lab is
    // In Task and Out of Task and we fire off an event when the state changes.
    AimlabLogState _state;
};

using AimlabLogWatcherPtr = std::unique_ptr<AimlabLogWatcher>;

}