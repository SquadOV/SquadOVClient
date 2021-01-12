#include "league/league_process_handler.h"

#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "game_event_watcher/league/league_log_watcher.h"
#include "api/league_ingame_api.h"
#include "shared/log/log.h"
#include "system/state.h"

#include <iostream>
#include <queue>
#include <unordered_set>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace service::league {

// League of Legends are unique in that they share the same game executable.
// So we can only actually know what game mode we're in once we actually connect to
// the game. Furthermore, League of Legends is unique in that the executable
// is launched only for the duration of the match.
class LeagueProcessHandlerInstance {
public:
    LeagueProcessHandlerInstance(const process_watcher::process::Process& p);
    ~LeagueProcessHandlerInstance();

    void onGameStart();
    void onGameEnd();

private:
    void onLeagueConfig(const shared::TimePoint& eventTime, const void* rawData);

    process_watcher::process::Process  _process;
    game_event_watcher::LeagueCommandLineCfg _cfg;
    service::api::LeagueIngameApiPtr _api;
    std::string _activePlayerSummonerName;
    shared::EGame _actualGame;

    service::recorder::GameRecorderPtr _recorder;
    game_event_watcher::LeagueLogWatcherPtr _logWatcher;
};

LeagueProcessHandlerInstance::LeagueProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p) {

    // The log watcher is for finding the log for finding extra information about
    // the game (namely things like the game ID and platform ID). Note that
    // information just needs to be collected before the game ends as that's when
    // we'll upload it.
    _logWatcher = std::make_unique<game_event_watcher::LeagueLogWatcher>(shared::nowUtc());
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ELeagueLogEvents::CommandLineCfg), std::bind(&LeagueProcessHandlerInstance::onLeagueConfig, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->loadFromExecutable(p.path());

    // The in-game API is used to query for information at the beginning of the match
    // to determine information about the match. We could use it to also update information
    // as time goes on but that's a giant TODO for now as we can just pull info from the
    // API endpoints at the end instead. We'll be using this info to determine what game
    // we're actually playing (LoL vs TFT). We also need to use the in-game API to determine
    // the current player. This information will be used to ensure we can determine which
    // user to do a backfill for (and to ensure that the account is owned by the current user)
    _api = std::make_unique<service::api::LeagueIngameApi>();

    const auto gameStats = _api->getGameStats();
    if (gameStats.gameMode == "TFT") {
        _actualGame = shared::EGame::TFT;
    } else {
        _actualGame = shared::EGame::LeagueOfLegends;
    }

    _activePlayerSummonerName = _api->getActivePlayerName();

    // Now we can create the recorder once we know what we're ACTUALLY playing.
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, _actualGame);
}

LeagueProcessHandlerInstance::~LeagueProcessHandlerInstance() {
}

void LeagueProcessHandlerInstance::onGameStart() {
    service::system::getGlobalState()->markGameRunning(_actualGame, true);
    _recorder->start();
}

void LeagueProcessHandlerInstance::onGameEnd() {
    service::system::getGlobalState()->markGameRunning(_actualGame, false);
    _recorder->stop();
}

void LeagueProcessHandlerInstance::onLeagueConfig(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* cfg = reinterpret_cast<const game_event_watcher::LeagueCommandLineCfg*>(rawData);
    if (!cfg) {
        LOG_WARNING("Null League Command Line Cfg?" << std::endl);
        return;
    }

    LOG_INFO("League Command Line Cfg: " << std::endl
        << "\tGame ID: " << cfg->gameId << std::endl
        << "\tRegion: " << cfg->region << std::endl
        << "\tPlatform: " << cfg->platformId << std::endl);
    _cfg = *cfg;
}

LeagueProcessHandler::LeagueProcessHandler() = default;
LeagueProcessHandler::~LeagueProcessHandler() = default;

void LeagueProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    // League already started - can ignore.
    if (_instance) {
        return;
    }

    LOG_INFO("START LEAGUE/TFT" << std::endl);
    _instance = std::make_unique<LeagueProcessHandlerInstance>(p);
    _instance->onGameStart();
}

void LeagueProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP LEAGUE/TFT" << std::endl);
    _instance->onGameEnd();
    _instance.reset(nullptr);
}

}