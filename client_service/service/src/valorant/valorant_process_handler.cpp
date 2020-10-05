#include "valorant/valorant_process_handler.h"

#include "game_event_watcher/valorant/valorant_log_watcher.h"
#include "valorant/valorant_api.h"
#include "valorant/valorant_match.h"
#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "database/api.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"

#include <iostream>
#include <queue>
#include <unordered_set>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace service::valorant {

class ValorantProcessHandlerInstance {
public:
    ValorantProcessHandlerInstance(const process_watcher::process::Process& p, const service::database::DatabaseApi* db);
    ~ValorantProcessHandlerInstance();

private:
    void onValorantMatchStart(const shared::TimePoint& eventTime, const void* rawData);
    void onValorantMatchEnd(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantBuyStart(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantRoundStart(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantRSOLogin(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantDetectPvpServer(const shared::TimePoint& eventTime,const void* rawData);

    void backfillMatchHistory();
    void cleanup();

    game_event_watcher::ValorantLogWatcherPtr _logWatcher;

    shared::riot::RiotUser _currentUser;

    ValorantApiPtr _api;
    std::string _pendingPvpServer;

    ValorantMatchPtr _currentMatch;
    const service::database::DatabaseApi* _db;
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

ValorantProcessHandlerInstance::ValorantProcessHandlerInstance(const process_watcher::process::Process& p, const service::database::DatabaseApi* db):
    _logWatcher(std::make_unique<game_event_watcher::ValorantLogWatcher>()),
    _db(db),
    _process(p) {

    // Initialize recorder to pull from the game window (but don't start recording until a game starts).
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::filesystem::getSquadOvRecordFolder(), shared::EGame::Valorant);

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
    cleanup();
}

ValorantProcessHandlerInstance::~ValorantProcessHandlerInstance() {
    _backfillRunning = false;
    _backfillThread.join();
}

void ValorantProcessHandlerInstance::backfillThreadJob() {
    while (_backfillRunning) {
        // Get their full match history and store each game's match details.
        try {
            std::string matchId;

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

            // If this match already exists on the database, make sure we merge so we don't lose our old data.
            ValorantMatchPtr existingMatch = _db->getValorantMatch(matchId);
            std::string existingVodPath;

            if (!!existingMatch) {
                if (!existingMatch->populateMatchDetailsFromApi(_api.get())) {
                    addMatchesToBackfill({matchId});
                }
                existingVodPath = _db->getVodFilenameForValorantMatch(matchId);
            } else {
                auto apiDetails = _api->getMatchDetails(matchId);
                existingMatch = std::make_unique<ValorantMatch>(std::move(*apiDetails));
            }

            _db->storeValorantMatch(existingMatch.get());
            if (!existingVodPath.empty()) {
                _db->associateValorantMatchToVideoFile(existingMatch->matchId(), existingVodPath);
            }
        } catch (const std::exception& e) {
            LOG_WARNING("Failed to backfill: " << e.what() << std::endl);
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

void ValorantProcessHandlerInstance::cleanup() {
    // Delete dangling videos that aren't associated with a match (maybe generated by games where we left or something).
    const fs::path dir = _recorder->outputFolder();
    for (const auto& f : fs::directory_iterator(dir)) {
        if (!_db->isValorantVideoAssociatedWithMatch(f.path().string())) {
            fs::remove(f.path());
        }
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
        _recorder->stop();
        _currentMatch.reset(nullptr);
    }

    _currentMatch = std::make_unique<ValorantMatch>(eventTime, state->matchMap, state->matchId);

    // When the match starts, we de facto start the first buy round too.
    onValorantBuyStart(eventTime, nullptr);
    _recorder->start(state->matchId);
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
        // Retrieve match details + our rank after the game (if relevant).
        if (!_currentMatch->populateMatchDetailsFromApi(_api.get())) {
            addMatchesToBackfill({_currentMatch->matchId()});
        }
        _currentMatch->finishMatch(eventTime);

        // Store match details.
        _db->storeValorantMatch(_currentMatch.get());

        // Associate the match with the video so that we know which video to load later
        // AND so we know which videos to not delete in our cleanup phase.
        _db->associateValorantMatchToVideoFile(_currentMatch->matchId(), _recorder->path().string());
    }

    _recorder->stop();

    // Reset the current match. If the check above fails, then this is the only thing that
    // happens which is fine since we don't care about the game anymore since something weird happened.
    _currentMatch.reset(nullptr);
}

void ValorantProcessHandlerInstance::backfillMatchHistory() {
    // If some exception is thrown and we fail to backfill just log it and move on since
    // we can always just try again later.
    try {      
        // If the total number of matches we have stored in the database doesn't match
        // the number of matches given to us by Riot's API then we know we aren't synced properly.
        size_t apiNumMatches = 0;
        _api->getLatestMatchId(_currentUser.puuid, &apiNumMatches);

        if (!apiNumMatches) {
            return;
        }

        const size_t dbNumMatches = _db->totalValorantMatchesForPuuid(_currentUser.puuid, true);
        if (dbNumMatches == apiNumMatches) {
            return;
        }

        // If the number of API matches don't match the number of matches we have stored in the database
        // then we know we need to do a sync. Figure out which matches are the ones that are different!
        std::vector<std::string> allApiMatchIds = _api->getMatchHistoryIds(_currentUser.puuid);
        std::vector<std::string> diffMatchIds;
        std::copy_if(allApiMatchIds.begin(), allApiMatchIds.end(), std::back_inserter(diffMatchIds), [this](const std::string& matchId){
            return !_db->isValorantMatchStored(matchId);
        });
        
        // Yes this can happen sometimes. I don't think Riot's API is very good at returning *every match* you've played in the match history
        // so it's possible that we'll sometimes get a match in the match history and then lose it some time later.
        if (diffMatchIds.empty()) {
            return;
        }
        
        LOG_INFO("Starting backfill [" << apiNumMatches << " API GAMES][" << dbNumMatches << " DB GAMES] of " << diffMatchIds.size() << " games." << std::endl);
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

ValorantProcessHandler::ValorantProcessHandler(const service::database::DatabaseApi* db):
    _db(db) {    
}
ValorantProcessHandler::~ValorantProcessHandler() {
}

void ValorantProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    // Valorant already started - can ignore.
    if (_instance) {
        return;
    }

    LOG_INFO("START VALORANT" << std::endl);
    _instance = std::make_unique<ValorantProcessHandlerInstance>(p, _db);
}

void ValorantProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP VALORANT" << std::endl);
    _instance.reset(nullptr);
}

}