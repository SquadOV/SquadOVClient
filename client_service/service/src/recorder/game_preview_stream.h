#pragma once

#include "shared/games.h"
#include "recorder/game_recorder.h"

#include <atomic>
#include <boost/variant.hpp>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <variant>

namespace service::recorder {

enum class GamePreviewTasksType {
    Start,
    Stop,
    Reload,
    EnableOverlay
};

struct GamePreviewStartData {
    std::string url;
    shared::EGame game;
};

struct GamePreviewTaskPacket {
    GamePreviewTasksType type;
    std::variant<GamePreviewStartData, bool, nullptr_t> data;
};

class GamePreviewStream;
class GamePreviewStreamThreadWorker {
public:
    explicit GamePreviewStreamThreadWorker(GamePreviewStream* stream);
    ~GamePreviewStreamThreadWorker();

    void queueWork(const GamePreviewTaskPacket& packet);

private:
    void worker();
    void handlePacket(const GamePreviewTaskPacket& packet) const;

    std::atomic<bool> _running = true;
    GamePreviewStream* _stream;

    mutable std::mutex _queueMutex;
    std::condition_variable _queueCv;
    std::deque<GamePreviewTaskPacket> _queue;

    std::thread _workerThread;
};

using GamePreviewStreamThreadWorkerPtr = std::unique_ptr<GamePreviewStreamThreadWorker>;

class GamePreviewStream {
public:
    GamePreviewStream();

    void start(const std::string& url, shared::EGame game);
    void stop();
    void reload();
    void enableOverlay(bool enable);

    friend class GamePreviewStreamThreadWorker;
private:

    void internalStart(const std::string& url, shared::EGame game);
    void internalStop();
    void internalReload();
    void internalEnableOverlay(bool enable);

    mutable std::mutex _mutex;
    bool _running = false;
    GameRecorderPtr _recorder;
    GamePreviewStreamThreadWorkerPtr _worker;
};

}