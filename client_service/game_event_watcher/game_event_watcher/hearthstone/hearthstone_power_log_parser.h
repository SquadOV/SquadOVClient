#pragma once
#include "game_event_watcher/hearthstone/hearthstone_raw_log.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace game_event_watcher {

class HearthstonePowerLogParser {
public:
    void parse(const HearthstoneRawLog& log);
    void clear();

    nlohmann::json toJson() const;
private:
    std::vector<HearthstoneRawLog> _raw;
};

}