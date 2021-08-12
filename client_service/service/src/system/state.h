#pragma once

#include "shared/games.h"
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

namespace service::system {

using GameStateChangeCallback = std::function<void(const shared::EGameSet&)>;

class GlobalState {
public:
    void setPause(bool p) {_paused = p; };
    bool isPaused() const { return _paused; };

    void setIsNvidiaGPU(bool p) { _nvidiaGpu = p; }
    bool isNvidiaGPU() const { return _nvidiaGpu; }

    void addGameRunningCallback(const GameStateChangeCallback& cb) { _runningCbs.push_back(cb); }
    void addGameRecordingCallback(const GameStateChangeCallback& cb) { _recordingCbs.push_back(cb); }

    void markGameRunning(shared::EGame game, bool running);
    void markGameRecording(shared::EGame game, bool recording);

private:
    bool _paused = false;
    bool _nvidiaGpu = false;
    shared::EGameSet _runningSet;
    std::vector<GameStateChangeCallback> _runningCbs;

    shared::EGameSet _recordingSet;
    std::vector<GameStateChangeCallback> _recordingCbs;
};

using GlobalStatePtr = std::unique_ptr<GlobalState>;

GlobalState* getGlobalState();

}