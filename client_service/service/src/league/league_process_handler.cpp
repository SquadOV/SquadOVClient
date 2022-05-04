#include "league/league_process_handler.h"

#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "game_event_watcher/league/league_log_watcher.h"
#include "api/league_ingame_api.h"
#include "api/squadov_api.h"
#include "shared/log/log.h"
#include "system/state.h"

#include <atomic>
#include <iostream>
#include <queue>
#include <mutex>
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
    void forceStopRecording();

private:
    void onLeagueConfig(const shared::TimePoint& eventTime, const void* rawData);
    void onLeagueLocalPlayer(const shared::TimePoint& eventTime, const void* rawData);

    void onLeagueAvailable(const shared::TimePoint& eventTime, const void* rawData);
    void onLeagueGameStart(const shared::TimePoint& eventTime, const void* rawData);

    void requestMatchCreation();
    void requestBackfill();

    bool isReady() const;

    process_watcher::process::Process  _process;
    game_event_watcher::LeagueCommandLineCfg _cfg;
    service::api::LeagueIngameApiPtr _api;
    service::api::LeagueIngameApiPollerPtr _apiWatcher;

    // When the game time is 0:00
    shared::TimePoint _gameStartTime = shared::TimePoint::max();
    bool _ownsAccount = false;
    std::string _currentMatchUuid;
    std::string _activePlayerSummonerName;
    shared::EGame _actualGame;

    mutable std::mutex _puuidMutex;
    std::condition_variable _puuidCv;
    std::string _localPuuid;

    mutable std::mutex _configMutex;
    bool _hasConfig = false;
    bool _hasInGame = false;
    bool _hasAccount = false;

    std::atomic_bool _matchCreated = false;
    std::atomic_bool _backfillRequested = false;

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
    _api = std::make_shared<service::api::LeagueIngameApi>();
    _apiWatcher = std::make_shared<service::api::LeagueIngameApiPoller>(_api);
    _apiWatcher->notifyOnEvent(static_cast<int>(service::api::ELeagueInGameEvent::Available), std::bind(&LeagueProcessHandlerInstance::onLeagueAvailable, this, std::placeholders::_1, std::placeholders::_2));
    _apiWatcher->notifyOnEvent(static_cast<int>(service::api::ELeagueInGameEvent::Start), std::bind(&LeagueProcessHandlerInstance::onLeagueGameStart, this, std::placeholders::_1, std::placeholders::_2));
    _apiWatcher->start();

    // The log watcher is for finding the log for finding extra information about
    // the game (namely things like the game ID and platform ID). Note that
    // information just needs to be collected before the game ends as that's when
    // we'll upload it.
    _logWatcher = std::make_unique<game_event_watcher::LeagueLogWatcher>(shared::nowUtc());
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ELeagueLogEvents::CommandLineCfg), std::bind(&LeagueProcessHandlerInstance::onLeagueConfig, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ELeagueLogEvents::LocalPlayer), std::bind(&LeagueProcessHandlerInstance::onLeagueLocalPlayer, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->loadFromExecutable(p.path());

    service::api::getGlobalApi()->markUserAnalyticsEvent("launch_game");
    service::api::getGlobalApi()->markUserAnalyticsEvent("launch_league");
}

LeagueProcessHandlerInstance::~LeagueProcessHandlerInstance() {
}

void LeagueProcessHandlerInstance::onGameStart() {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    LOG_INFO("On LoL/TFT Game Start" << std::endl);
}

void LeagueProcessHandlerInstance::onGameEnd() {
    service::system::getGlobalState()->markGameRunning(_actualGame, false);

    if (service::system::getGlobalState()->isPaused() || _currentMatchUuid.empty()) {
        return;
    }

    LOG_INFO("On LoL/TFT Game End: " << _currentMatchUuid << std::endl);

    if (_recorder->isRecording()) {
        const auto vodId = _recorder->currentId();
    
        std::optional<service::recorder::GameRecordEnd> end = std::nullopt;
        if (!_currentMatchUuid.empty() && _ownsAccount) {
            try {
                if (_actualGame == shared::EGame::TFT) {
                    service::api::getGlobalApi()->finishTftMatch(_currentMatchUuid);
                } else {
                    service::api::getGlobalApi()->finishLeagueOfLegendsMatch(_currentMatchUuid);
                }
                end = std::make_optional(service::recorder::GameRecordEnd{
                    _currentMatchUuid,
                    shared::nowUtc()
                });
            }  catch (std::exception& ex) {
                LOG_WARNING("Failed to upload LoL/TFT match: " << ex.what() << std::endl);
                try {
                    service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
                } catch (std::exception& ex) {
                    LOG_WARNING("Failed to delete VOD: " << ex.what());            
                }
            }
        }

        _recorder->stop(end);
    }
    
    _currentMatchUuid.clear();
}

void LeagueProcessHandlerInstance::forceStopRecording() {
    _currentMatchUuid.clear();
    if (_recorder->isRecording()) {
        _recorder->stop({});
    }
}

void LeagueProcessHandlerInstance::onLeagueAvailable(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    const auto* stats = reinterpret_cast<const service::api::LeagueGameStats*>(rawData);
    if (!stats) {
        LOG_WARNING("Null League game stats on available?" << std::endl);
        return;
    }

    LOG_INFO("League In-Game API Available: " << shared::timeToStr(eventTime) << std::endl 
        << "\tGame Mode: " << stats->gameMode << std::endl
        << "\tGame Time: " << stats->gameTime << std::endl
        << "\tMap Name: " << stats->mapName << std::endl
        << "\tMap Number: " << stats->mapNumber << std::endl
    );

    if (stats->gameMode == "TFT") {
        _actualGame = shared::EGame::TFT;
    } else {
        _actualGame = shared::EGame::LeagueOfLegends;
    }
    service::system::getGlobalState()->markGameRunning(_actualGame, true);

    // There's no guarantee that the getActivePlayerName endpoint will return a non-empty string right away
    // so start a thread to start watching for that as soon as the in-game API is available to us.
    std::thread t([this, eventTime]() {
        while (_activePlayerSummonerName.empty()) {
            try {
                _activePlayerSummonerName = _api->getActivePlayerName();
                LOG_INFO("Retrieved Summoner Name: " << _activePlayerSummonerName << std::endl);
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to get active summoner name: " << ex.what() << std::endl);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::unique_lock<std::mutex> lock(_puuidMutex);
        _puuidCv.wait(lock, [this](){ return !_localPuuid.empty(); });
        LOG_INFO("Obtained local PUUID: " << _localPuuid << std::endl);

        if (_actualGame == shared::EGame::TFT) {
            _ownsAccount = service::api::getGlobalApi()->verifyTftAccountOwnership(_activePlayerSummonerName, _localPuuid, _cfg.platformId);
        } else {
            _ownsAccount = service::api::getGlobalApi()->verifyLeagueOfLegendsAccountOwnership(_activePlayerSummonerName, _localPuuid, _cfg.platformId);
        }
        
        LOG_INFO("\tDetermined Game: " << shared::gameToString(_actualGame) << std::endl);
        LOG_INFO("\tOwned Account: " << _ownsAccount << std::endl);
        {
            std::unique_lock<std::mutex> guard(_configMutex);
            _hasAccount = true;
        }

        if (_ownsAccount) {
            _recorder->start(eventTime, service::recorder::RecordingMode::Normal);
        }

        requestMatchCreation();
        requestBackfill();
    });
    t.detach();
    
    // Now we can create the recorder once we know what we're ACTUALLY playing.
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, _actualGame);
}

void LeagueProcessHandlerInstance::onLeagueGameStart(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    LOG_INFO("On League Game Start: " << shared::timeToStr(eventTime) << std::endl);
    _gameStartTime = eventTime;
    
    {
        std::unique_lock<std::mutex> guard(_configMutex);
        _hasInGame = true;
    }

    requestMatchCreation();
    requestBackfill();
}

void LeagueProcessHandlerInstance::onLeagueLocalPlayer(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    const auto* player = reinterpret_cast<const game_event_watcher::LeagueLocalPlayer*>(rawData);
    if (!player) {
        LOG_WARNING("Null League Local Player?" << std::endl);
        return;
    }

    LOG_INFO("League Local Player: " << std::endl
        << "\tName: " << player->name << std::endl
        << "\tPUUID: " << player->puuid << std::endl);
    
    {
        std::lock_guard<std::mutex> guard(_puuidMutex);
        _localPuuid = player->puuid;
    }
    _puuidCv.notify_all();
}

void LeagueProcessHandlerInstance::onLeagueConfig(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
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

    {
        std::unique_lock<std::mutex> guard(_configMutex);
        _hasConfig = true;
    }

    requestMatchCreation();
    requestBackfill();
}

bool LeagueProcessHandlerInstance::isReady() const {
    std::unique_lock<std::mutex> guard(_configMutex);
    return _hasInGame && _hasConfig && _hasAccount;
}

void LeagueProcessHandlerInstance::requestMatchCreation() {
    // There a couple checks here.
    // 1) isReady checks for whether onLeagueConfig, onLeagueAvailable, and onLeagueGameStart have all been called.
    //    Note that the flag checked in isReady is only ticked in onLeagueConfig and onLeagueConfig. There's no call
    //    in onLeagueAvailable because onLeagueGameStart is guaranteed to be called after onLeagueAvailable in the
    //    same thread.
    // 2) Atomic set and check on _matchCreated (for backfill it'd be _backfillRequested) to make sure we only try to create a match once.
    if (!isReady() || !_ownsAccount) {
        LOG_INFO("Skipping match creation either due to not ready or not owned account." << std::endl);
        return;
    }

    const auto prev = _matchCreated.exchange(true);
    if (prev) {
        return;
    }

    LOG_INFO("Requesting LoL/TFT match creation..." << std::endl);

    // This is where we create the match UUID on the server as we can only get
    // a unique Riot-based identifier for the match (region/platform + game ID)
    // once this event fires.
    const auto flags = service::api::getGlobalApi()->getSessionFeatures();
    try {
        if (_actualGame == shared::EGame::TFT) {
            LOG_INFO("...Creating TFT." << std::endl);
            _currentMatchUuid = service::api::getGlobalApi()->createNewTftMatch(_cfg.region, _cfg.platformId, _cfg.gameId, _gameStartTime);
        } else {
            LOG_INFO("...Creating LoL." << std::endl);
            _currentMatchUuid = service::api::getGlobalApi()->createNewLeagueOfLegendsMatch(_cfg.platformId, _cfg.gameId, _gameStartTime);
        }
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create new LoL/TFT match: " << ex.what() << std::endl);
    }
}

void LeagueProcessHandlerInstance::requestBackfill() {
    if (!isReady() || !_ownsAccount) {
        LOG_INFO("Skipping backfill either due to not ready or not owned account." << std::endl);
        return;
    }

    const auto prev = _backfillRequested.exchange(true);
    if (prev) {
        return;
    }

    LOG_INFO("Requesting LoL/TFT backfill..." << std::endl);

    const auto flags = service::api::getGlobalApi()->getSessionFeatures();
    try {
        // Note that it's the server's responsibility to make sure we don't backfill too often.
        if (_actualGame == shared::EGame::TFT) {
            LOG_INFO("...Backfill TFT." << std::endl);
            service::api::getGlobalApi()->requestTftBackfill(_activePlayerSummonerName, _cfg.region);
        } else {
            LOG_INFO("...Backfill LoL." << std::endl);
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

void LeagueProcessHandler::forceStopRecording() {
    if (!_instance) {
        return;
    }
    LOG_INFO("Force Stop Recording: LEAGUE/TFT" << std::endl);
    _instance->forceStopRecording();
}

}