#include "system/state.h"

namespace service::system {

GlobalState* getGlobalState() {
    static GlobalStatePtr global = std::make_unique<GlobalState>();
    return global.get();
}

void GlobalState::markGameRunning(shared::EGame game, bool running) {
    if (running) {
        _runningSet.insert(game);
    } else {
        _runningSet.erase(game);
    }

    for (const auto& cb: _runningCbs) {
        cb(_runningSet);
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