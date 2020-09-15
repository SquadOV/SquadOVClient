#include "valorant/valorant_process_handler.h"

#include "game_event_watcher/valorant/valorant_log_watcher.h"
#include "shared/time.h"

#include <iostream>

namespace service::valorant {

class ValorantProcessHandlerInstance {
public:
    ValorantProcessHandlerInstance();
    ~ValorantProcessHandlerInstance();

private:
    void onValorantMatchStart(const shared::TimePoint& eventTime, const void* rawData);
    void onValorantMatchEnd(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantBuyStart(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantRoundStart(const shared::TimePoint& eventTime,const void* rawData);
    void onValorantRSOLogin(const shared::TimePoint& eventTime,const void* rawData);

    game_event_watcher::ValorantLogWatcherPtr _logWatcher;
};

ValorantProcessHandlerInstance::ValorantProcessHandlerInstance():
    _logWatcher(std::make_unique<game_event_watcher::ValorantLogWatcher>()) {

    // Initialize Valorant data. Pull in account information and populate
    // match history/account data in the database as needed.
    _logWatcher->notifyOnEvent(game_event_watcher::EValorantLogEvents::RSOLogin, std::bind(&ValorantProcessHandlerInstance::onValorantRSOLogin, this, std::placeholders::_1, std::placeholders::_2));

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

    // Initialize streamer to pull from the game window (but don't start recording until a game starts).
}

ValorantProcessHandlerInstance::~ValorantProcessHandlerInstance() {
}

void ValorantProcessHandlerInstance::onValorantMatchStart(const shared::TimePoint& eventTime, const void*) {
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Match Start" << std::endl;
}

void ValorantProcessHandlerInstance::onValorantMatchEnd(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::GameLogState*>(rawData);
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Match End" << std::endl
        << "\tMap: " << shared::valorant::mapToName(state->matchMap) << std::endl
        << "\tMatchID: " << state->matchId << std::endl
        << "\tAPI Server: " << state->apiServer << std::endl;
}

void ValorantProcessHandlerInstance::onValorantBuyStart(const shared::TimePoint& eventTime, const void*) {
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Round BUY Start" << std::endl;
}

void ValorantProcessHandlerInstance::onValorantRoundStart(const shared::TimePoint& eventTime, const void*) {
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant Round PLAY Start" << std::endl;
}

void ValorantProcessHandlerInstance::onValorantRSOLogin(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::ClientLogState*>(rawData);
    std::cout << "[" << shared::timeToStr(eventTime) << "] Valorant RSO Login" << std::endl;
    std::cout << "\tUser: " << state->user.username << "#" << state->user.tag << std::endl;
    std::cout << "\tPUUID: " << state->user.puuid << std::endl;
    std::cout << "\tRSO Token: " << state->rsoToken << std::endl;
    std::cout << "\tEntitlement Token: " << state->entitlementToken << std::endl;
}

ValorantProcessHandler::ValorantProcessHandler() = default;
ValorantProcessHandler::~ValorantProcessHandler() = default;

void ValorantProcessHandler::onProcessStarts() {
    // Valorant already started - can ignore.
    if (_instance) {
        return;
    }

    std::cout << "START VALORANT" << std::endl;
    _instance = std::make_unique<ValorantProcessHandlerInstance>();
}

void ValorantProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    std::cout << "STOP VALORANT" << std::endl;
    _instance.reset(nullptr);
}

}