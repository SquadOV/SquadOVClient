#pragma once

#include "shared/games.h"
#include "shared/time.h"
#include <functional>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace service::system {

using GameStateChangeCallback = std::function<void(const shared::EGameSet&)>;
using GameStatusCallback = std::function<void(shared::EGame, bool, const shared::TimePoint&)>;

class GlobalState {
public:
    void setPause(bool p) {_paused = p; };
    bool isPaused() const { return _paused; };

    void setIsNvidiaGPU(bool p) { _nvidiaGpu = p; }
    bool isNvidiaGPU() const { return _nvidiaGpu; }

    void addGameRunningCallback(const GameStateChangeCallback& cb) { _runningCbs.push_back(cb); }
    void addGameRecordingCallback(const GameStateChangeCallback& cb) { _recordingCbs.push_back(cb); }
    void addGameStatusCallback(const GameStatusCallback& cb) { _statusCbs.push_back(cb); }

    void markGameRunning(shared::EGame game, bool running);
    void markGameRecording(shared::EGame game, bool recording);

    bool areGamesRunning() const { return !_runningSet.empty(); }
private:
    bool _paused = false;
    bool _nvidiaGpu = false;
    shared::EGameSet _runningSet;
    std::unordered_map<shared::EGame, shared::TimePoint> _runningStarts;
    std::vector<GameStateChangeCallback> _runningCbs;

    shared::EGameSet _recordingSet;
    std::vector<GameStateChangeCallback> _recordingCbs;

    std::vector<GameStatusCallback> _statusCbs;
};

using GlobalStatePtr = std::unique_ptr<GlobalState>;

GlobalState* getGlobalState();

}