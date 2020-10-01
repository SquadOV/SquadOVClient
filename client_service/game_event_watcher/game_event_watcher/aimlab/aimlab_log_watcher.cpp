#include "game_event_watcher/aimlab/aimlab_log_watcher.h"

#include <iostream>

#include "shared/strings/strings.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"

namespace fs = std::filesystem;

namespace game_event_watcher {
namespace {

const std::string startTaskPrefix = "-------START-TASK-------";
bool parseStartTask(const std::string& line) {
    return shared::strings::hasPrefix(line, startTaskPrefix);
}

const std::string taskNamePrefix = "Task Name: ";
bool parseTaskName(const std::string& line, std::string& taskName) {
    if (!shared::strings::hasPrefix(line, taskNamePrefix)) {
        return false;
    }

    taskName = line.substr(taskNamePrefix.size());
    return true;
}

const std::string taskModePrefix = "Task Mode: ";
bool parseTaskMode(const std::string& line, std::string& taskMode) {
    if (!shared::strings::hasPrefix(line, taskModePrefix)) {
        return false;
    }

    taskMode = line.substr(taskModePrefix.size());
    return true;
}

const std::string taskMapPrefix = "Task Map: ";
bool parseTaskMap(const std::string& line, std::string& taskMap) {
    if (!shared::strings::hasPrefix(line, taskMapPrefix)) {
        return false;
    }

    taskMap = line.substr(taskMapPrefix.size());
    return true;
}

const std::string gameVersionPrefix = "Build Version: ";
bool parseGameVersion(const std::string& line, std::string& version) {
    if (!shared::strings::hasPrefix(line, gameVersionPrefix)) {
        return false;
    }

    version = line.substr(gameVersionPrefix.size());
    return true;
}

const std::string finishTaskPrefix = "-------PERFORMANCE-------";
bool parseFinishTask(const std::string& line) {
    return shared::strings::hasPrefix(line, finishTaskPrefix);
}

const std::string unloadTaskPrefix = "UnloadTime";
bool parseUnloadTask(const std::string& line) {
    return shared::strings::hasPrefix(line, unloadTaskPrefix);
}

}

bool operator==(const AimlabLogState& a, const AimlabLogState& b) {
    return (a.inTask == b.inTask) &&
        (a.taskName == b.taskName) &&
        (a.taskMode == b.taskMode) &&
        (a.taskMap == b.taskMap) &&
        (a.gameVersion == b.gameVersion);
}

bool AimlabLogState::isInTask() const {
    return inTask && !taskName.empty() && !taskMode.empty() && !taskMap.empty() && !gameVersion.empty();
}

AimlabLogWatcher::AimlabLogWatcher() {
    // Find log file which is stored at (Windows):
    //  %APPDATA%/../LocalLow/Statespace/aimlab_tb/Player.log
    const fs::path gameLogDir = shared::filesystem::getAimlabAppDataFolder();

    _logPath = gameLogDir / fs::path("Player.log");
    LOG_INFO("AIM LAB Game Log: " << _logPath.string() << std::endl);

    using std::placeholders::_1;
    _watcher = std::make_unique<LogWatcher>(_logPath, std::bind(&AimlabLogWatcher::onGameLogChange, this, _1), true);
}

void AimlabLogWatcher::onGameLogChange(const LogLinesDelta& lines) {
    const AimlabLogState previousState = _state;
    bool restartFlag = false;
    bool killFlag = false;

    for (const auto& ln : lines) {
        if (parseStartTask(ln)) {
            _state.inTask = true;
            if (previousState.isInTask()) {
                restartFlag = true;
            }
            continue;
        } else if (_state.inTask && parseTaskMode(ln, _state.taskMode)) {
            continue;
        } else if (_state.inTask && parseTaskMap(ln, _state.taskMap)) {
            continue;
        } else if (_state.inTask && parseTaskName(ln, _state.taskName)) {
            continue;
        } else if (_state.inTask && parseGameVersion(ln, _state.gameVersion)) {
            continue;            
        } else if (parseFinishTask(ln)) {
            _state = AimlabLogState{};
            continue;
        } else if (parseUnloadTask(ln)) {
            // Force the task to go into a stopped state.
            // SEt the kill flag since we need the consumer
            // to know that they should discard the task that they
            // currently think they're tracking.
            _state = AimlabLogState{};
            killFlag = true;
            continue;
        }
    }

    // If the state didn't change then we know that we didn't parse anything of importance.
    if (!restartFlag && ((previousState == _state) || (previousState.isInTask() == _state.isInTask()))) {
        return;
    }

    // We've detected a change! We just need to detect changes in the return of
    // 'isInTask' (rather than the inTask boolean) to make sure we've picked up all
    // the information about being in a game.
    if (killFlag) {
        notify(static_cast<int>(EAimlabLogEvents::KillTask), shared::nowUtc(), (void*)&previousState);
    } else if (restartFlag) {
        notify(static_cast<int>(EAimlabLogEvents::RestartTask), shared::nowUtc(), (void*)&_state);
    } else if (_state.isInTask()) {
        notify(static_cast<int>(EAimlabLogEvents::StartTask), shared::nowUtc(), (void*)&_state);
    } else {
        notify(static_cast<int>(EAimlabLogEvents::FinishTask), shared::nowUtc(), (void*)&previousState);
    }
}

}