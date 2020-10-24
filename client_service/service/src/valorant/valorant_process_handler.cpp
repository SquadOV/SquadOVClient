#include "valorant/valorant_process_handler.h"

#include "game_event_watcher/valorant/valorant_log_watcher.h"
#include "valorant/valorant_api.h"
#include "valorant/valorant_match.h"
#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "local/local_data.h"
#include "api/squadov_api.h"

#include <iostream>
#include <queue>
#include <unordered_set>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace service::valorant {

class ValorantProcessHandlerInstance {
public:
    ValorantProcessHandlerInstance(const process_watcher::process::Process& p);
    ~ValorantProcessHandlerInstance();

private:
    void onValorantMatchStart(const shared::TimePoint& eventTime, const void* rawData);
    void onValorantMatchEnd(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantBuyStart(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantRoundStart(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantRSOLogin(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantDetectPvpServer(const shared::TimePoint& eventTime,const void* rawData);

    void backfillMatchHistory();

    game_event_watcher::ValorantLogWatcherPtr _logWatcher;

    shared::riot::RiotUser _currentUser;

    ValorantApiPtr _api;
    std::string _pendingPvpServer;

    ValorantMatchPtr _currentMatch;
    process_watcher::process::Process  _process;

    service::recorder::GameRecorderPtr _recorder;

    // Backfill thread related. We need to have a separate backfill thread running
    // constantly because that's where requests for match details will go if
    // we just so happen to be unable to get match details as soon as the game ends.
    void backfillThreadJob();
    void addMatchesToBackfill(const std::vector<std::string>& matchIds);

    bool _backfillRunning = true;
    std::thread _backfillThread;
    std::mutex _backfillMutex;
    std::queue<std::string> _backfillQueue;
};

ValorantProcessHandlerInstance::ValorantProcessHandlerInstance(const process_watcher::process::Process& p):
    _logWatcher(std::make_unique<game_event_watcher::ValorantLogWatcher>()),
    _process(p) {

    // Initialize recorder to pull from the game window (but don't start recording until a game starts).
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::Valorant);

    // Initialize Valorant data. Pull in account information and populate
    // match history/account data in the database as needed.
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::RSOLogin), std::bind(&ValorantProcessHandlerInstance::onValorantRSOLogin, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::PvpServer), std::bind(&ValorantProcessHandlerInstance::onValorantDetectPvpServer, this, std::placeholders::_1, std::placeholders::_2));

    // Create a Valorant game event detector.
    // Need to detect
    //  1) Game Start
    //  2) Game End
    //  3) Round Start (Buy)
    //  4) Round Start (Play)
    // This is what we use to start/stop recording.
    // When the game end, we retrieve stats and such from the Valorant API.
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::MatchStart), std::bind(&ValorantProcessHandlerInstance::onValorantMatchStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::MatchEnd), std::bind(&ValorantProcessHandlerInstance::onValorantMatchEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::RoundBuyStart), std::bind(&ValorantProcessHandlerInstance::onValorantBuyStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::RoundPlayStart), std::bind(&ValorantProcessHandlerInstance::onValorantRoundStart, this, std::placeholders::_1, std::placeholders::_2));

    // Start the backfill thread.
    _backfillThread = std::thread(std::bind(&ValorantProcessHandlerInstance::backfillThreadJob, this));
}

ValorantProcessHandlerInstance::~ValorantProcessHandlerInstance() {
    _backfillRunning = false;
    _backfillThread.join();
}

void ValorantProcessHandlerInstance::backfillThreadJob() {
    while (_backfillRunning) {
        // Get their full match history and store each game's match details.
        std::string matchId;
        try {

            {
                std::unique_lock<std::mutex> guard(_backfillMutex);
                if (!_backfillQueue.empty()) {
                    matchId = _backfillQueue.front();
                    _backfillQueue.pop();
                }
            }

            if (matchId.empty()) {
                // Don't want to constantly have the lock.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            auto apiDetails = _api->getRawMatchDetails(matchId);
            service::api::getGlobalApi()->uploadValorantMatch(matchId, apiDetails, nlohmann::json());
        } catch (const std::exception& e) {
            LOG_WARNING("Failed to backfill: " << e.what() << std::endl);
            addMatchesToBackfill({ matchId });
        }

        // Give it a hot sec before continuing just so we aren't constantly looping around even if
        // we just failed.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ValorantProcessHandlerInstance::addMatchesToBackfill(const std::vector<std::string>& matchIds) {
    std::unique_lock<std::mutex> guard(_backfillMutex);
    for (const auto& id : matchIds) {
        LOG_INFO("Adding match to backfill queue: " << id << std::endl);
        _backfillQueue.push(id);
    }
}

void ValorantProcessHandlerInstance::onValorantDetectPvpServer(const shared::TimePoint& eventTime,const void* rawData) {
    const auto* server = reinterpret_cast<const std::string*>(rawData);
    if (!!_api) {
        _api->initializePvpServer(*server);
        backfillMatchHistory();
    } else {
        _pendingPvpServer = *server;
    }
}

void ValorantProcessHandlerInstance::onValorantMatchStart(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::GameLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant Match Start" << std::endl
        << "\tMap: " << shared::valorant::mapToName(state->matchMap) << std::endl
        << "\tMatchID: " << state->matchId << std::endl
        << "\tAPI Server: " << state->apiServer << std::endl);

    // Don't record on unsupported maps (e.g. the range).
    if (!shared::valorant::isGameMap(state->matchMap)) {
        return;
    }

    // In the case where we're starting a match again without clearing out the current match
    // we *may* be dealing with a case of a custom game restart. In that case we should just completely restart EVERYTHING
    // and just pretend that the old match never happened.
    if (!!_currentMatch && _currentMatch->matchId() == state->matchId) {
        service::recorder::VodIdentifier id = _recorder->currentId();
        _recorder->stop();
        _currentMatch.reset(nullptr);
        service::api::getGlobalApi()->deleteVod(id.videoUuid);
    }

    _currentMatch = std::make_unique<ValorantMatch>(eventTime, state->matchMap, state->matchId);

    // When the match starts, we de facto start the first buy round too.
    onValorantBuyStart(eventTime, nullptr);
    _recorder->start();
}

void ValorantProcessHandlerInstance::onValorantMatchEnd(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::GameLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant Match End" << std::endl
        << "\tMap: " << shared::valorant::mapToName(state->matchMap) << std::endl
        << "\tMatchID: " << state->matchId << std::endl
        << "\tAPI Server: " << state->apiServer << std::endl);

    if (!_currentMatch || !_api) {
        LOG_WARNING("\tUNEXPECTED MATCH END (match or api nullptr)." << std::endl);
        return;
    }

    // Make sure we're still referring to the same game before trying to
    // fill in the match details. Also note that we don't try to pull or store
    // match details if the user just leaves the game.
    // Don't try to to check the the state's match map matches at this point since it'll probably
    // be back at the main menu before we get this event (as expected).
    if (_currentMatch->matchId() == state->matchId && state->stagedMatchEnd) {
        _currentMatch->finishMatch(eventTime);

        const auto& vodId = _recorder->currentId();

        try {
            // Store match details. Upload match without match details first just to get the match populated.
            // Grab the match via backfill later.
            const auto matchUuid = service::api::getGlobalApi()->uploadValorantMatch(
                _currentMatch->matchId(),
                nlohmann::json(),
                _currentMatch->toJson(_currentUser.puuid)
            );

            // Retrieve match details + our rank after the game (if relevant).
            addMatchesToBackfill({_currentMatch->matchId()});
            
            // Associate the match with the video so that we know which video to load later
            // AND so we know which videos to not delete in our cleanup phase.
            shared::squadov::VodAssociation association;
            association.matchUuid = matchUuid;
            association.userUuid = vodId.userUuid;
            association.videoUuid = vodId.videoUuid;
            association.startTime = _currentMatch->startTime();
            association.endTime = _currentMatch->endTime();
            service::api::getGlobalApi()->associateVod(association, _recorder->getMetadata());
        } catch (...) {
            // Any errors should result in the VOD being deleted.
            service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
        }
    }

    _recorder->stop();

    // Reset the current match. If the check above fails, then this is the only thing that
    // happens which is fine since we don't care about the game anymore since something weird happened.
    _currentMatch.reset(nullptr);
}

void ValorantProcessHandlerInstance::backfillMatchHistory() {
    // If some exception is thrown and we fail to backfill just log it and move on since
    // we can always just try again later. It's tempting to put in something in the local
    // data to track when the last backfill time was. However that won't work in a way
    // that's super reliable because of the fact that the backfill happens on a separate thread.
    // Let's say we have matches A, B, and C (A is the oldest match, C is the newest match).
    // The obvious way to do track the backfill time is that once we successfully pull the match
    // and upload it to the API server, we mark the game's start time as the "last backfill time."
    // However, the way the backfill works is to process all the games in the queue even if a
    // game fails. This means that if A and C successfully process but B fails, then the next time
    // we go to backfill, we'll have C's match time as the last backfill time and thus won't
    // pickup B again. Thus, the only alternative is the query all the user's matches every time
    // they start VALORANT to see if any backfill tasks are necessary.
    try {      
        // Pull all the matches that VALORANT's API feels like giving us. Send to the server
        // and return the match IDs that we need to fetch and upload.
        std::vector<std::string> allApiMatchIds = _api->getMatchHistoryIds(_currentUser.puuid);

        // I believe the valorant API returns match IDs in descending date order (most recent first).
        // We want to backfill from oldest to newest so that even if some backfill task fails, we'll
        // still be able to pick it up next time around.
        std::reverse(allApiMatchIds.begin(), allApiMatchIds.end());

        std::vector<std::string> diffMatchIds = service::api::getGlobalApi()->obtainMissingValorantMatches(allApiMatchIds);
        
        // Yes this can happen sometimes. I don't think Riot's API is very good at returning *every match* you've played in the match history
        // so it's possible that we'll sometimes get a match in the match history and then lose it some time later.
        if (diffMatchIds.empty()) {
            return;
        }
        
        LOG_INFO("Starting backfill of " << diffMatchIds.size() << " VALORANT matches." << std::endl);
        addMatchesToBackfill(diffMatchIds);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to perform Valorant backfill: " << ex.what() << std::endl);
    }
}

void ValorantProcessHandlerInstance::onValorantBuyStart(const shared::TimePoint& eventTime, const void*) {
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant Round BUY Start" << std::endl);
    if (!_currentMatch) {
        LOG_WARNING("\tUNEXPECTED ROUND BUY (match nullptr)." << std::endl);
        return;
    }
    _currentMatch->goToRoundState(eventTime, shared::valorant::EValorantRoundState::Buy);
}

void ValorantProcessHandlerInstance::onValorantRoundStart(const shared::TimePoint& eventTime, const void*) {
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant Round PLAY Start" << std::endl);
    if (!_currentMatch) {
        LOG_WARNING("\tUNEXPECTED ROUND START (match nullptr)." << std::endl);
        return;
    }
    _currentMatch->goToRoundState(eventTime, shared::valorant::EValorantRoundState::Play);
}

void ValorantProcessHandlerInstance::onValorantRSOLogin(const shared::TimePoint& eventTime, const void* rawData) {
    // When we detect this login event, we only get the PUUID. We'll need to query our own API to get an RSO token and entitlements token
    // to use.
    const auto* puuid = reinterpret_cast<const std::string*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant RSO Login" << std::endl
        << "\tPUUID: " << *puuid << std::endl);


    shared::riot::RiotRsoToken token;
    try {
        token = service::api::getGlobalApi()->refreshValorantRsoToken(*puuid);
    } catch (std::exception& ex) {
        LOG_ERROR("Failed to refresh Valorant RSO token: " << ex.what());
        // If we failed to get the RSO token we'll just assume the user will restart the game.
        return;
    }

    // Now that we have the RSO/Entitlement token we know enough to start querying the API.
    // Only need to give the token to the API object once as the object will take care of
    // auto-refreshing the token for us....
    if (!_api) {
        _api = std::make_unique<ValorantApi>(token);
        if (!_pendingPvpServer.empty()) {
            _api->initializePvpServer(_pendingPvpServer);
            backfillMatchHistory();
            _pendingPvpServer = "";
        }
    }

    _currentUser = service::api::getGlobalApi()->getRiotUserFromPuuid(*puuid);
}

ValorantProcessHandler::ValorantProcessHandler() = default;
ValorantProcessHandler::~ValorantProcessHandler() = default;

void ValorantProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    // Valorant already started - can ignore.
    if (_instance) {
        return;
    }

    LOG_INFO("START VALORANT" << std::endl);
    _instance = std::make_unique<ValorantProcessHandlerInstance>(p);
}

void ValorantProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP VALORANT" << std::endl);
    _instance.reset(nullptr);
}

}