#include "game_event_watcher/hearthstone/hearthstone_power_log_parser.h"
#include <regex>

namespace game_event_watcher {

const std::string BEGIN_MULLIGAN = "tag=MULLIGAN_STATE value=DONE";
const std::string GAMEOVER_TAG = "TAG_CHANGE Entity=GameEntity tag=STEP value=FINAL_GAMEOVER";
const std::string BLOCKEND = "Block End=(null)";

void HearthstonePowerLogParser::parse(const HearthstoneRawLog& log) {
    _raw.push_back(log);

    if (log.log.find(BEGIN_MULLIGAN) != std::string::npos) {
        // This will happen twice - one for each player but that's fine.
        // We use the mulligan done log line for detecting game start as that's
        // when we will reliably know that the user has actually loaded into the game.
        // TODO: It probably won't work for other game modes.
        _gameRunning = true;
    } else if (log.log.find(GAMEOVER_TAG) != std::string::npos) {
        _endPending = true;
    } else if (_endPending && log.log.find(BLOCKEND) != std::string::npos) {
        _gameRunning = false;
        _endPending = false;
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