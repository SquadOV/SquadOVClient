#pragma once

#include "shared/games.h"
#include "recorder/game_recorder.h"

#include <mutex>

namespace service::recorder {

class GamePreviewStream {
public:
    void start(const std::string& url, shared::EGame game);
    void stop();
    void reload();
    void enableOverlay(bool enable);

private:
    mutable std::mutex _mutex;
    bool _running = false;
    GameRecorderPtr _recorder;
};

}