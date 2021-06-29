#include "recorder/game_preview_stream.h"
#include "process_watcher/watcher.h"

#include "shared/log/log.h"

#include <chrono>
#include <VersionHelpers.h>

namespace service::recorder {

GamePreviewStreamThreadWorker::GamePreviewStreamThreadWorker(GamePreviewStream* stream):
    _stream(stream)
{
    _workerThread = std::thread([this](){
        this->worker();
    });
}

void GamePreviewStreamThreadWorker::queueWork(const GamePreviewTaskPacket& packet) {
    std::lock_guard guard(_queueMutex);
    _queue.push_back(packet);
    _queueCv.notify_one();
}

void GamePreviewStreamThreadWorker::worker() {
    while (_running) {
        std::unique_lock guard(_queueMutex);
        if (!_queueCv.wait_for(guard, std::chrono::milliseconds(100), [this](){
            return !_queue.empty();
        })) {
            continue;
        }

        while (!_queue.empty()) {
            const auto& packet = _queue.front();
            handlePacket(packet);
            _queue.pop_front();
        }
    }
}

void GamePreviewStreamThreadWorker::handlePacket(const GamePreviewTaskPacket& packet) const {
    switch (packet.type) {
        case GamePreviewTasksType::Start:
        {
            const auto& data = std::get<GamePreviewStartData>(packet.data);
            _stream->internalStart(data.url, data.game);
            break;
        }
        case GamePreviewTasksType::Stop:
        {
            _stream->internalStop();
            break;
        }
        case GamePreviewTasksType::Reload:
        {
            _stream->internalReload();
            break;
        }
        case GamePreviewTasksType::EnableOverlay:
        {
            const auto& enabled = std::get<bool>(packet.data);
            _stream->internalEnableOverlay(enabled);
            break;
        }
    }
}

GamePreviewStreamThreadWorker::~GamePreviewStreamThreadWorker() {

}

GamePreviewStream::GamePreviewStream():
    _worker(new GamePreviewStreamThreadWorker(this))
{
}

void GamePreviewStream::start(const std::string& url, shared::EGame game) {
    GamePreviewTaskPacket packet;
    packet.type = GamePreviewTasksType::Start;

    GamePreviewStartData start;
    start.url = url;
    start.game = game;
    packet.data = start;

    _worker->queueWork(packet);
}

void GamePreviewStream::stop() {
    GamePreviewTaskPacket packet;
    packet.type = GamePreviewTasksType::Stop;
    packet.data = nullptr;
    _worker->queueWork(packet);
}

void GamePreviewStream::reload() {
    GamePreviewTaskPacket packet;
    packet.type = GamePreviewTasksType::Reload;
    packet.data = nullptr;
    _worker->queueWork(packet);
}

void GamePreviewStream::enableOverlay(bool enable) {
    GamePreviewTaskPacket packet;
    packet.type = GamePreviewTasksType::EnableOverlay;
    packet.data = enable;
    _worker->queueWork(packet);
}

void GamePreviewStream::internalStart(const std::string& url, shared::EGame game) {
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

void GamePreviewStream::internalStop() {
    std::lock_guard guard(_mutex);
    if (!_running) {
        return;
    }

    _recorder->stop({});
    _recorder.reset();
    _running = false;
}

void GamePreviewStream::internalReload() {
    std::lock_guard guard(_mutex);
    _recorder->stop({});
    _recorder->start(shared::nowUtc(), service::recorder::RecordingMode::Normal);
}

void GamePreviewStream::internalEnableOverlay(bool enable) {
    std::lock_guard guard(_mutex);
    _recorder->enableOverlay(enable);
}

}