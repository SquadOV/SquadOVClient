#include "game_event_watcher/hearthstone/hearthstone_power_log_parser.h"
#include <regex>

namespace game_event_watcher {

const std::string CREATE_GAME_DELIMITER = "GameState.DebugPrintPower() - CREATE_GAME";
const std::string END_TASK_LIST = "PowerProcessor.EndCurrentTaskList()";

const std::regex goldRewardRegex("PowerTaskList.DebugPrintPower\\(\\) -\\s*TAG_CHANGE.*tag=GOLD_REWARD_STATE value=1");
bool parseGoldRewardState(const std::string& log) {
    return std::regex_search(log, goldRewardRegex);
}

void HearthstonePowerLogParser::parse(const HearthstoneRawLog& log) {
    _raw.push_back(log);

    if (log.log.find(CREATE_GAME_DELIMITER) != std::string::npos) {
        _gameRunning = true;
    } else if (parseGoldRewardState(log.log)) {
        _endPending = true;
    } else if (log.log.find(END_TASK_LIST) != std::string::npos) {
        if (_endPending) {
            _gameRunning = false;
            _endPending = false;
        }
    }
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