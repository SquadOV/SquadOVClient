#include "recorder/game_preview_stream.h"
#include "process_watcher/watcher.h"

#include "shared/log/log.h"
#include <VersionHelpers.h>

namespace service::recorder {

void GamePreviewStream::start(const std::string& url, shared::EGame game) {
    std::lock_guard guard(_mutex);
    if (_running) {
        return;
    }
    auto gameProcess = process_watcher::isGameRunning(game);
    if (!gameProcess) {
        LOG_INFO("Trying to start game preview stream for a game that isn't running?" << std::endl);
        return;
    }

    _recorder = std::make_unique<GameRecorder>(gameProcess.value(), game);
    _recorder->forceUrl(url);
    _recorder->start(shared::nowUtc(), service::recorder::RecordingMode::Normal);
    _running = true;

}

void GamePreviewStream::stop() {
    std::lock_guard guard(_mutex);
    if (!_running) {
        return;
    }

    _recorder->stop({});
    _recorder.reset();
    _running = false;
}

void GamePreviewStream::reload() {
    std::lock_guard guard(_mutex);
    _recorder->stop({});
    _recorder->start(shared::nowUtc(), service::recorder::RecordingMode::Normal);
}

void GamePreviewStream::enableOverlay(bool enable) {
    std::lock_guard guard(_mutex);
    _recorder->enableOverlay(enable);
}

}