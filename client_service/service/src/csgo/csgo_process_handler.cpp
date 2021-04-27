#include "csgo/csgo_process_handler.h"
#include "game_event_watcher/csgo/csgo_log_watcher.h"
#include "game_event_watcher/csgo/csgo_gsi_state_manager.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "system/state.h"

#include <mutex>
namespace service::csgo {

class CsgoProcessHandlerInstance {
public:
    CsgoProcessHandlerInstance(const process_watcher::process::Process& p);
    ~CsgoProcessHandlerInstance();

private:
    game_event_watcher::CsgoLogWatcherPtr _logWatcher;
    game_event_watcher::CsgoGsiStateManager _stateManager;

    process_watcher::process::Process  _process;
    service::recorder::GameRecorderPtr _recorder;

    void onGsiMatchStart(const shared::TimePoint& eventTime, const void* rawData);
    void onGsiMatchEnd(const shared::TimePoint& eventTime, const void* rawData);

    void onLogConnect(const shared::TimePoint& eventTime, const void* rawData);
    void onLogDisconnect(const shared::TimePoint& eventTime, const void* rawData);

    void onSquadOvMatchStart(const shared::TimePoint& eventTime);

    // There's two things that need to be set by two different threads for us to
    // officially declare us as being either in game or out of game.
    //    1) The last recorded CsgoMatchState.
    //    2) The connected server.
    // If both are set then we're actively in a game and if both are not set then
    // we are no longer in a game.
    std::recursive_mutex _activeGameMutex;
    std::optional<game_event_watcher::CsgoMatchState> _matchState;
    std::optional<std::string> _serverIdentifier;
    std::optional<std::string> _activeViewUuid;
};

CsgoProcessHandlerInstance::CsgoProcessHandlerInstance(const process_watcher::process::Process& p):
    _logWatcher(std::make_unique<game_event_watcher::CsgoLogWatcher>(shared::nowUtc())),
    _process(p) {
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::CSGO);

    _stateManager.notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoGsiEvents::MatchStart), std::bind(&CsgoProcessHandlerInstance::onGsiMatchStart, this, std::placeholders::_1, std::placeholders::_2));
    _stateManager.notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoGsiEvents::MatchEnd), std::bind(&CsgoProcessHandlerInstance::onGsiMatchEnd, this, std::placeholders::_1, std::placeholders::_2));

    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoLogEvents::Connect), std::bind(&CsgoProcessHandlerInstance::onLogConnect, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoLogEvents::Disconnect), std::bind(&CsgoProcessHandlerInstance::onLogDisconnect, this, std::placeholders::_1, std::placeholders::_2));
}

CsgoProcessHandlerInstance::~CsgoProcessHandlerInstance() {

}

void CsgoProcessHandlerInstance::onGsiMatchStart(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    std::lock_guard guard(_activeGameMutex);

    const auto* state = reinterpret_cast<const game_event_watcher::CsgoMatchState*>(rawData);
    if (!state) {
        LOG_WARNING("Error in retrieving CSGO match state for GSI start." << std::endl);
        return;
    }

    _matchState = *state;
    onSquadOvMatchStart(eventTime);
}

void CsgoProcessHandlerInstance::onLogConnect(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    std::lock_guard guard(_activeGameMutex);

    const auto* server = reinterpret_cast<const std::string*>(rawData);
    if (!server) {
        LOG_WARNING("Error in retrieving CSGO server for log connect." << std::endl);
        return;
    }

    _serverIdentifier = *server;
    onSquadOvMatchStart(eventTime);
}

void CsgoProcessHandlerInstance::onSquadOvMatchStart(const shared::TimePoint& eventTime) {
    std::lock_guard guard(_activeGameMutex);
    if (!_matchState || !_serverIdentifier) {
        return;
    }

    if (_activeViewUuid) {
        LOG_WARNING("Trying to start a new CSGO match when one is already active?" << std::endl);
        return;
    }

    try {
        _activeViewUuid = service::api::getGlobalApi()->createNewCsgoMatch(_serverIdentifier.value(), eventTime, _matchState->map, _matchState->mode);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create CS:GO match: " << ex.what() << std::endl);
        return;
    }

    // At this point we know that we can successfully request a unique match UUID
    // for the match we're in and that we can start recording. The server and the
    // time of day should be sufficient to uniquely identify the match. We need to wait
    // for the GSI match start anyway to collect aux information that we'll use to
    // further identify the match.
    _recorder->start(eventTime, service::recorder::RecordingMode::Normal);
}

void CsgoProcessHandlerInstance::onGsiMatchEnd(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    std::lock_guard guard(_activeGameMutex);
    _matchState.reset();

    if (!_activeViewUuid) {
        LOG_WARNING("Trying to start end a CSGO match when none are active?" << std::endl);
        return;
    }

    const auto* state = reinterpret_cast<const game_event_watcher::CsgoMatchState*>(rawData);
    if (!state) {
        LOG_WARNING("Error in retrieving CSGO match state for GSI end." << std::endl);
        return;
    }

    std::optional<service::recorder::GameRecordEnd> end = std::nullopt;

    const auto vodId = _recorder->currentId();
    try {
        // TODO: Retrieve the proper demo URL.
        service::api::getGlobalApi()->finishCsgoMatch(_activeViewUuid.value(), eventTime, *state, "");

        end = std::make_optional(service::recorder::GameRecordEnd{
            _activeViewUuid.value(),
            eventTime
        });
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to upload CS:GO match: " << ex.what() << std::endl);
        try {
            service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to delete VOD: " << ex.what());            
        }
    }

    if (_recorder->isRecording()) {
        _recorder->stop(end);
    }

    _activeViewUuid.reset();
}

void CsgoProcessHandlerInstance::onLogDisconnect(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    std::lock_guard guard(_activeGameMutex);

    // A log disconnect should NOT take us out of a game state.
    // We want to wait for the GSI to take care of that event as it'll
    // bring CSGO back to the menu which our GSI state manager is smart
    // enough to identify as being the same as a 'game over.' This is
    // purely to prevent us from starting another match without re-connecting
    // to a server.
    _serverIdentifier.reset();
}

CsgoProcessHandler::CsgoProcessHandler() = default;
CsgoProcessHandler::~CsgoProcessHandler() = default;

void CsgoProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }
    LOG_INFO("START CS:GO" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::CSGO, true);
    _instance = std::make_unique<CsgoProcessHandlerInstance>(p);
}

void CsgoProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP CS:GO" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::CSGO, false);
    _instance.reset(nullptr);
}

}