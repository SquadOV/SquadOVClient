#include "system/state.h"

namespace service::system {

GlobalState* getGlobalState() {
    static GlobalStatePtr global = std::make_unique<GlobalState>();
    return global.get();
}

void GlobalState::markGameRunning(shared::EGame game, bool running) {
    shared::TimePoint startTime;

    if (running) {
        _runningSet.insert(game);
        _runningStarts[game] = shared::nowUtc();
    } else {
        _runningSet.erase(game);

        startTime = _runningStarts[game];
        _runningStarts.erase(game);
    }

    for (const auto& cb: _runningCbs) {
        cb(_runningSet);
    }

    for (const auto& cb: _statusCbs) {
        cb(game, running, startTime);
    }
}

void GlobalState::markGameRecording(shared::EGame game, bool recording) {
    if (recording) {
        _recordingSet.insert(game);
    } else {
        _recordingSet.erase(game);
    }

    for (const auto& cb: _recordingCbs) {
        cb(_recordingSet);
    }
}

}