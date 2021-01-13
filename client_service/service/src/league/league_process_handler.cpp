#include "league/league_process_handler.h"

#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "game_event_watcher/league/league_log_watcher.h"
#include "api/league_ingame_api.h"
#include "api/squadov_api.h"
#include "shared/log/log.h"
#include "system/state.h"

#include <iostream>
#include <queue>
#include <unordered_set>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace service::league {

// League of Legends and TFT are unique in that they share the same game executable.
// So we can only actually know what game mode we're in once we actually connect to
// the game. Furthermore, these games are unique in that the executable
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

    shared::TimePoint _startTime;
    bool _ownsAccount = false;
    std::string _currentMatchUuid;
    std::string _activePlayerSummonerName;
    shared::EGame _actualGame;

    service::recorder::GameRecorderPtr _recorder;
    game_event_watcher::LeagueLogWatcherPtr _logWatcher;
};

LeagueProcessHandlerInstance::LeagueProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p) {
    // The in-game API is used to query for information at the beginning of the match
    // to determine information about the match. We could use it to also update information
    // as time goes on but that's a giant TODO for now as we can just pull info from the
    // API endpoints at the end instead. We'll be using this info to determine what game
    // we're actually playing (LoL vs TFT). We also need to use the in-game API to determine
    // the current player. This information will be used to ensure we can determine which
    // user to do a backfill for (and to ensure that the account is owned by the current user)
    _api = std::make_unique<service::api::LeagueIngameApi>();
    try {
        _activePlayerSummonerName = _api->getActivePlayerName();
        LOG_INFO("Retrieved Summoner Name: " << _activePlayerSummonerName << std::endl);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to get active summoner name: " << ex.what() << std::endl);
        return;
    }

    try {
        const auto gameStats = _api->getGameStats();
        if (gameStats.gameMode == "TFT") {
            _actualGame = shared::EGame::TFT;
            _ownsAccount = service::api::getGlobalApi()->verifyTftAccountOwnership(_activePlayerSummonerName);
        } else {
            _actualGame = shared::EGame::LeagueOfLegends;
            _ownsAccount = service::api::getGlobalApi()->verifyLeagueOfLegendsAccountOwnership(_activePlayerSummonerName);
        }
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to get game mode or verify account ownership: " << ex.what() << std::endl);
        return;
    }

    LOG_INFO("Determined Game: " << shared::gameToString(_actualGame) << std::endl);
    LOG_INFO("Owned Account: " << _ownsAccount << std::endl);

    // The log watcher is for finding the log for finding extra information about
    // the game (namely things like the game ID and platform ID). Note that
    // information just needs to be collected before the game ends as that's when
    // we'll upload it.
    _logWatcher = std::make_unique<game_event_watcher::LeagueLogWatcher>(shared::nowUtc());
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ELeagueLogEvents::CommandLineCfg), std::bind(&LeagueProcessHandlerInstance::onLeagueConfig, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->loadFromExecutable(p.path());

    // Now we can create the recorder once we know what we're ACTUALLY playing.
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, _actualGame);
}

LeagueProcessHandlerInstance::~LeagueProcessHandlerInstance() {
}

void LeagueProcessHandlerInstance::onGameStart() {
    service::system::getGlobalState()->markGameRunning(_actualGame, true);

    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }
    
    _startTime = shared::nowUtc();
    _recorder->start();
}

void LeagueProcessHandlerInstance::onGameEnd() {
    service::system::getGlobalState()->markGameRunning(_actualGame, false);

    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }

    const auto vodId = _recorder->currentId();
    const auto sessionId = _recorder->sessionId();
    const auto metadata = _recorder->getMetadata();
    _recorder->stop();

    if (!_currentMatchUuid.empty()) {
        try {
            if (_actualGame == shared::EGame::TFT) {
                service::api::getGlobalApi()->finishTftMatch(_currentMatchUuid);
            } else {
                service::api::getGlobalApi()->finishLeagueOfLegendsMatch(_currentMatchUuid);
            }

            shared::squadov::VodAssociation association;
            association.matchUuid = _currentMatchUuid;
            association.userUuid = vodId.userUuid;
            association.videoUuid = vodId.videoUuid;
            association.startTime = _startTime;
            association.endTime = shared::nowUtc();
            service::api::getGlobalApi()->associateVod(association, metadata, sessionId);
        }  catch (std::exception& ex) {
            LOG_WARNING("Failed to upload LoL/TFT match: " << ex.what() << std::endl);
            try {
                service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to delete VOD: " << ex.what());            
            }
        }
    }
    
    _currentMatchUuid.clear();
}

void LeagueProcessHandlerInstance::onLeagueConfig(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }

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

    // This is where we create the match UUID on the server as we can only get
    // a unique Riot-based identifier for the match (region/platform + game ID)
    // once this event fires.
    try {
        if (_actualGame == shared::EGame::TFT) {
            _currentMatchUuid = service::api::getGlobalApi()->createNewTftMatch(_cfg.region, _cfg.platformId, _cfg.gameId);
        } else {
            _currentMatchUuid = service::api::getGlobalApi()->createNewLeagueOfLegendsMatch(_cfg.platformId, _cfg.gameId);
        }
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create new LoL/TFT match: " << ex.what() << std::endl);
    }

    try {
        // Note that it's the server's responsibility to make sure we don't backfill too often.
        if (_actualGame == shared::EGame::TFT) {
            service::api::getGlobalApi()->requestTftBackfill(_activePlayerSummonerName, _cfg.region);
        } else {
            service::api::getGlobalApi()->requestLeagueOfLegendsBackfill(_activePlayerSummonerName, _cfg.platformId);
        }
    } catch (std::exception& ex) {
        // Not a real error as it doesn't get in the way of us doing anything.
        LOG_WARNING("Failed to request LoL/TFT backfill: " << ex.what() << std::endl);
    }
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