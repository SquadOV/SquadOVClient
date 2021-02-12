#pragma once
#include "game_event_watcher/hearthstone/hearthstone_raw_log.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace game_event_watcher {

class HearthstonePowerLogParser {
public:
    void parse(const HearthstoneRawLog& log);
    void postParse();

    void clear();

    nlohmann::json toJson() const;
    bool isGameRunning() const { return _gameRunning; }

private:
    std::vector<HearthstoneRawLog> _raw;
    bool _gameRunning = false;
    bool _endPending = false;
    bool _clearOnPost = false;
};

}