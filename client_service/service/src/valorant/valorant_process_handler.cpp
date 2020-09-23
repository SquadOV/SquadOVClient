#include "valorant/valorant_process_handler.h"

#include "game_event_watcher/valorant/valorant_log_watcher.h"
#include "valorant/valorant_api.h"
#include "valorant/valorant_match.h"
#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "database/api.h"
#include "recorder/game_recorder.h"

#include <atomic>
#include <iostream>

using namespace std::chrono_literals;

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

    game_event_watcher::ValorantLogWatcherPtr _logWatcher;

    shared::riot::RiotUser _currentUser;

    ValorantApiPtr _api;
    std::string _pendingPvpServer;

    ValorantMatchPtr _currentMatch;
    const service::database::DatabaseApi* _db;
    std::atomic_bool _isCurrentlyBackfilling = false;
    process_watcher::process::Process  _process;

    service::recorder::GameRecorderPtr _recorder;
};

ValorantProcessHandlerInstance::ValorantProcessHandlerInstance(const process_watcher::process::Process& p, const service::database::DatabaseApi* db):
    _logWatcher(std::make_unique<game_event_watcher::ValorantLogWatcher>()),
    _db(db),
    _process(p) {
    // Initialize recorder to pull from the game window (but don't start recording until a game starts).
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::filesystem::getSquadOvRecordFolder(), shared::EGame::Valorant);

    // Initialize Valorant data. Pull in account information and populate
    // match history/account data in the database as needed.
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::RSOLogin, std::bind(&ValorantProcessHandlerInstance::onValorantRSOLogin, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::PvpServer, std::bind(&ValorantProcessHandlerInstance::onValorantDetectPvpServer, this, std::placeholders::_1, std::placeholders::_2));

    // Create a Valorant game event detector.
    // Need to detect
    //  1) Game Start
    //  2) Game End
    //  3) Round Start (Buy)
    //  4) Round Start (Play)
    // This is what we use to start/stop recording.
    // When the game end, we retrieve stats and such from the Valorant API.
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::MatchStart, std::bind(&ValorantProcessHandlerInstance::onValorantMatchStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::MatchEnd, std::bind(&ValorantProcessHandlerInstance::onValorantMatchEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::RoundBuyStart, std::bind(&ValorantProcessHandlerInstance::onValorantBuyStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::RoundPlayStart, std::bind(&ValorantProcessHandlerInstance::onValorantRoundStart, this, std::placeholders::_1, std::placeholders::_2));
}

ValorantProcessHandlerInstance::~ValorantProcessHandlerInstance() {
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
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Match Start" << std::endl
        << "\tMap: " << shared::valorant::mapToName(state->matchMap) << std::endl
        << "\tMatchID: " << state->matchId << std::endl
        << "\tAPI Server: " << state->apiServer << std::endl;

    // Don't record on unsupported maps (e.g. the range).
    if (!shared::valorant::isGameMap(state->matchMap)) {
        return;
    }

    _currentMatch = std::make_unique<ValorantMatch>(eventTime, state->matchMap, state->matchId);

    // When the match starts, we de facto start the first buy round too.
    onValorantBuyStart(eventTime, nullptr);
    _recorder->start(state->matchId);
}

void ValorantProcessHandlerInstance::onValorantMatchEnd(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::GameLogState*>(rawData);
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Match End" << std::endl
        << "\tMap: " << shared::valorant::mapToName(state->matchMap) << std::endl
        << "\tMatchID: " << state->matchId << std::endl
        << "\tAPI Server: " << state->apiServer << std::endl;

    if (!_currentMatch || !_api) {
        std::cerr << "\tUNEXPECTED MATCH END (match or api nullptr)." << std::endl;
        return;
    }
    
    // Make sure we're still referring to the same game before trying to
    // fill in the match details.
    if (_currentMatch->matchId() == state->matchId && _currentMatch->map() == state->matchMap) {
        // Retrieve match details + our rank after the game (if relevant).
        _currentMatch->populateMatchDetailsFromApi(_api.get());
        _currentMatch->finishMatch(eventTime);

        // Store match details.
        _db->storeValorantMatch(_currentMatch.get());
    }

    // Reset the current match. If the check above fails, then this is the only thing that
    // happens which is fine since we don't care about the game anymore since something weird happened.
    _currentMatch.reset(nullptr);
    _recorder->stop();
}

void ValorantProcessHandlerInstance::backfillMatchHistory() {
    // If the total number of matches we have stored in the database doesn't match
    // the number of matches given to us by Riot's API then we know we aren't synced properly.
    size_t apiNumMatches = 0;
    _api->getLatestMatchId(_currentUser.puuid, &apiNumMatches);

    if (!apiNumMatches) {
        return;
    }

    const size_t dbNumMatches = _db->totalValorantMatchesForPuuid(_currentUser.puuid);
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
    
    // If _isCurrentlyBackfilling is true, then compare_exchange_strong returns false so we can return
    // because a backfill task is already in progress. If _isCurrentlyBackfilling is false, the exchange
    // happens and the function returns true.
    bool isBackfilling = false;
    if (!_isCurrentlyBackfilling.compare_exchange_strong(isBackfilling, true)) {
        return;
    }

    std::thread backFillThread([this, diffMatchIds](){
        // Get their full match history and store each game's match details.
        try {
            for (const auto& matchId : diffMatchIds) {
                ValorantMatch match(std::move(*_api->getMatchDetails(matchId)));
                _db->storeValorantMatch(&match);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to backfill: " << e.what() << std::endl;
        }
    });
    backFillThread.detach();
}

void ValorantProcessHandlerInstance::onValorantBuyStart(const shared::TimePoint& eventTime, const void*) {
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Round BUY Start" << std::endl;
    if (!_currentMatch) {
        std::cerr << "\tUNEXPECTED ROUND BUY (match nullptr)." << std::endl;
        return;
    }
    _currentMatch->goToRoundState(eventTime, shared::valorant::EValorantRoundState::Buy);
}

void ValorantProcessHandlerInstance::onValorantRoundStart(const shared::TimePoint& eventTime, const void*) {
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Round PLAY Start" << std::endl;
    if (!_currentMatch) {
        std::cerr << "\tUNEXPECTED ROUND START (match nullptr)." << std::endl;
        return;
    }
    _currentMatch->goToRoundState(eventTime, shared::valorant::EValorantRoundState::Play);
}

void ValorantProcessHandlerInstance::onValorantRSOLogin(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::ClientLogState*>(rawData);
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant RSO Login" << std::endl;
    std::cout << "\tUser: " << state->user.username << "#" << state->user.tag << std::endl;
    std::cout << "\tPUUID: " << state->user.puuid << std::endl;
    std::cout << "\tRSO Token: " << state->rsoToken << std::endl;
    std::cout << "\tEntitlement Token: " << state->entitlementToken << std::endl;

    // Now that we have the RSO/Entitlement token we know enough to start querying the API.
    _currentUser = state->user;

    if (!_api) {
        _api = std::make_unique<ValorantApi>(state->rsoToken, state->entitlementToken);
        if (!_pendingPvpServer.empty()) {
            _api->initializePvpServer(_pendingPvpServer);
            backfillMatchHistory();
            _pendingPvpServer = "";
        }
    } else {
        _api->reinitTokens(state->rsoToken, state->entitlementToken);

        // Might need to do a backfill if the user has been running VALORANT for a long time
        // and the first attempt at trying to backfill failed using an old token.
        backfillMatchHistory();
    }

    // Should store this user in the database if they don't exist already.
    _db->storeValorantAccount(_currentUser);
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

    std::cout << "START VALORANT" << std::endl;
    _instance = std::make_unique<ValorantProcessHandlerInstance>(p, _db);
}

void ValorantProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    std::cout << "STOP VALORANT" << std::endl;
    _instance.reset(nullptr);
}

}