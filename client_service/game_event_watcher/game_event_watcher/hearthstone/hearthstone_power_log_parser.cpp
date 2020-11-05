#include "game_event_watcher/hearthstone/hearthstone_power_log_parser.h"

namespace game_event_watcher {

void HearthstonePowerLogParser::parse(const HearthstoneRawLog& log) {
    _raw.push_back(log);
}

void HearthstonePowerLogParser::clear() {
    _raw.clear();
}

nlohmann::json HearthstonePowerLogParser::toJson() const {
    nlohmann::json ret = nlohmann::json::array();
    for (const auto& l : _raw) {
        ret.push_back(nlohmann::json{
            { "time", shared::timeToIso(l.tm) },
            { "section", l.section },
            { "log", l.log }
        });
    }
    return ret;
}

}