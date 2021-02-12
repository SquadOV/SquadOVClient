#include "game_event_watcher/hearthstone/hearthstone_power_log_parser.h"
#include "shared/log/log.h"
#include <regex>

namespace game_event_watcher {

const std::string BEGIN_MULLIGAN = "tag=MULLIGAN_STATE value=DONE";
const std::string GAMEOVER_TAG = "TAG_CHANGE Entity=GameEntity tag=STATE value=COMPLETE";
const std::string BLOCKEND = "Block End=(null)";

void HearthstonePowerLogParser::parse(const HearthstoneRawLog& log) {
    _raw.push_back(log);

    if (log.log.find(BEGIN_MULLIGAN) != std::string::npos) {
        LOG_INFO("Hearthstone Mark Game Running" << std::endl);
        // This will happen twice - one for each player but that's fine.
        // We use the mulligan done log line for detecting game start as that's
        // when we will reliably know that the user has actually loaded into the game.
        // TODO: It probably won't work for other game modes.
        _gameRunning = true;
    } else if (log.log.find(GAMEOVER_TAG) != std::string::npos) {
        LOG_INFO("Hearthstone Mark End Pending" << std::endl);
        _endPending = true;
    } else if (_endPending && log.log.find(BLOCKEND) != std::string::npos) {
        LOG_INFO("Hearthstone End" << std::endl);
        _gameRunning = false;
        _endPending = false;
        _clearOnPost = true;
    }
}

void HearthstonePowerLogParser::postParse() {
    // We need to clear the power log parser once the current game is finished. We can't
    // do it in the parse() function because the log watcher will still need to collect that
    // info to send to the server at that point. Hence, we must force the log watcher to call
    // the post parse function for us to safely deal with things after the watcher has no
    // more use for the data. We can't have the log watcher call clear for us because
    // the log watcher has no visibility on when the end was detected (only whether the
    // current running state). If we only try to clear when the state changes, then we run
    // the risk of collecting data from previous matches if for whatever odd reason the
    // user decides to restart SquadOV in the middle of a match. If we try to always clear when the
    // state is false (game is not running), then we won't capture certain important events like
    // the CREATE_GAME in the beginning. Thus, we must detect when we'd set the _gameRunning flag
    // to false in parse() and manually mark a flag to note that we should be clearing the
    // buffer in postParse to successfully keep the power log buffer clear of power logs that
    // do not belong to the current game.
    if (_clearOnPost) {
        _clearOnPost = false;
        clear();
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