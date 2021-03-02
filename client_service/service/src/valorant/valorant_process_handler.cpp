#include "valorant/valorant_process_handler.h"

#include "game_event_watcher/valorant/valorant_log_watcher.h"
#include "valorant/valorant_match.h"
#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "local/local_data.h"
#include "api/squadov_api.h"
#include "system/state.h"

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

    void backfillMatchHistory();

    game_event_watcher::ValorantLogWatcherPtr _logWatcher;

    bool _ownsAccount = false;
    shared::riot::RiotUser _currentUser;
    ValorantMatchPtr _currentMatch;
    process_watcher::process::Process  _process;
    shared::TimePoint _vodStartTime;

    service::recorder::GameRecorderPtr _recorder;
};

ValorantProcessHandlerInstance::ValorantProcessHandlerInstance(const process_watcher::process::Process& p):
    _logWatcher(std::make_unique<game_event_watcher::ValorantLogWatcher>(shared::nowUtc())),
    _process(p) {

    // Initialize recorder to pull from the game window (but don't start recording until a game starts).
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::Valorant);

    // Initialize Valorant data. Pull in account information and populate
    // match history/account data in the database as needed.
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EValorantLogEvents::RSOLogin), std::bind(&ValorantProcessHandlerInstance::onValorantRSOLogin, this, std::placeholders::_1, std::placeholders::_2));

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
}

ValorantProcessHandlerInstance::~ValorantProcessHandlerInstance() {
}

void ValorantProcessHandlerInstance::onValorantMatchStart(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }

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
        _recorder->stop({});
        _currentMatch.reset(nullptr);
        try {
            service::api::getGlobalApi()->deleteVod(id.videoUuid);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to delete VOD: " << ex.what());            
        }
    }

    _currentMatch = std::make_unique<ValorantMatch>(eventTime, state->matchMap, state->matchId);

    // When the match starts, we de facto start the first buy round too.
    onValorantBuyStart(eventTime, nullptr);
    _recorder->start(eventTime, service::recorder::RecordingMode::Normal);
}

void ValorantProcessHandlerInstance::onValorantMatchEnd(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }

    const auto* state = reinterpret_cast<const game_event_watcher::GameLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant Match End" << std::endl
        << "\tMap: " << shared::valorant::mapToName(state->matchMap) << std::endl
        << "\tMatchID: " << state->matchId << std::endl
        << "\tAPI Server: " << state->apiServer << std::endl);

    if (!_currentMatch) {
        LOG_WARNING("\tUNEXPECTED MATCH END (match nullptr)." << std::endl);
        return;
    }

    // Make sure we're still referring to the same game before trying to
    // fill in the match details. Also note that we don't try to pull or store
    // match details if the user just leaves the game.
    // Don't try to to check the the state's match map matches at this point since it'll probably
    // be back at the main menu before we get this event (as expected).
    std::optional<service::recorder::GameRecordEnd> end = std::nullopt;

    if (_currentMatch->matchId() == state->matchId && state->stagedMatchEnd) {
        _currentMatch->finishMatch(eventTime);

        const auto vodId = _recorder->currentId();
        const auto sessionId = _recorder->sessionId();
        const auto vodStartTime = _recorder->vodStartTime();

        try {
            // Store match details. Upload match without match details first just to get the match populated.
            // Grab the match via backfill later.
            const auto matchUuid = service::api::getGlobalApi()->uploadValorantMatch(
                _currentMatch->matchId(),
                _currentUser.puuid,
                _currentMatch->toJson()
            );
            
            // Associate the match with the video so that we know which video to load later
            // AND so we know which videos to not delete in our cleanup phase.
            end = std::make_optional(service::recorder::GameRecordEnd{
                matchUuid,
                _currentMatch->endTime()
            });
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to upload valorant match: " << ex.what() << std::endl);
            // Any errors should result in the VOD being deleted.
            try {
                service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to delete VOD: " << ex.what());            
            }
        }
    }

    if (_recorder->isRecording()) {
        _recorder->stop(end);
    }

    // Reset the current match. If the check above fails, then this is the only thing that
    // happens which is fine since we don't care about the game anymore since something weird happened.
    _currentMatch.reset(nullptr);
}

void ValorantProcessHandlerInstance::backfillMatchHistory() {
    if (!_ownsAccount) {
        return;
    }
    service::api::getGlobalApi()->requestValorantMatchBackfill(_currentUser.puuid);
}

void ValorantProcessHandlerInstance::onValorantBuyStart(const shared::TimePoint& eventTime, const void*) {
    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }

    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant Round BUY Start" << std::endl);
    if (!_currentMatch) {
        LOG_WARNING("\tUNEXPECTED ROUND BUY (match nullptr)." << std::endl);
        return;
    }
    _currentMatch->goToRoundState(eventTime, shared::valorant::EValorantRoundState::Buy);
}

void ValorantProcessHandlerInstance::onValorantRoundStart(const shared::TimePoint& eventTime, const void*) {
    if (service::system::getGlobalState()->isPaused() || !_ownsAccount) {
        return;
    }

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
    const auto* user = reinterpret_cast<const shared::riot::RiotUser*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Valorant RSO Login" << std::endl
        << "\tPUUID: " << user->puuid << std::endl
        << "\tUsername: " << user->username << std::endl
        << "\tTagline: " << user->tag << std::endl);

    _ownsAccount = service::api::getGlobalApi()->verifyValorantAccountOwnership(user->username, user->tag, user->puuid);
    LOG_INFO("Owns Account: " << _ownsAccount << std::endl);
    if (!_ownsAccount) {
        return;
    }

    _currentUser = *user;

    try {
        backfillMatchHistory();
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to perform Valorant match backfill: " << ex.what() << std::endl);
    }
}

ValorantProcessHandler::ValorantProcessHandler() = default;
ValorantProcessHandler::~ValorantProcessHandler() = default;

void ValorantProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    // Valorant already started - can ignore.
    if (_instance) {
        return;
    }

    LOG_INFO("START VALORANT" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Valorant, true);
    _instance = std::make_unique<ValorantProcessHandlerInstance>(p);
}

void ValorantProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP VALORANT" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Valorant, false);
    _instance.reset(nullptr);
}

}