#include "csgo/csgo_process_handler.h"
#include "game_event_watcher/csgo/csgo_log_watcher.h"
#include "game_event_watcher/csgo/csgo_gsi_state_manager.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "system/state.h"
#include "shared/ipc/shared_memory.h"
#include "shared/uuid.h"
#include "shared/process.h"
#include "shared/filesystem/common_paths.h"
#include "shared/base64/decode.h"
#include <boost/algorithm/string.hpp>

#include <mutex>

namespace fs = std::filesystem;
namespace service::csgo {

struct DemoRequest {
    std::string viewUuid;
    shared::TimePoint threshold;
};

class CsgoProcessHandlerInstance {
public:
    CsgoProcessHandlerInstance(const process_watcher::process::Process& p);
    ~CsgoProcessHandlerInstance();

    void forceStopRecording();
private:
    game_event_watcher::CsgoLogWatcherPtr _logWatcher;
    game_event_watcher::CsgoGsiStateManager _stateManager;

    process_watcher::process::Process  _process;
    service::recorder::GameRecorderPtr _recorder;

    void onGsiMatchStart(const shared::TimePoint& eventTime, const void* rawData);
    void onGsiMatchEnd(const shared::TimePoint& eventTime, const void* rawData);
    void onGsiMainMenu(const shared::TimePoint& eventTime, const void* rawData);

    void onLogConnect(const shared::TimePoint& eventTime, const void* rawData);
    void onLogDisconnect(const shared::TimePoint& eventTime, const void* rawData);

    void onSquadOvMatchStart(const shared::TimePoint& eventTime);

    // There's two things that need to be set by two different threads for us to
    // officially declare us as being either in game or out of game.
    //    1) The last recorded CsgoMatchState.
    //    2) The connected server.
    // If both are set then we're actively in a game and if both are not set then
    // we are no longer in a game. However, it's possible start a new match without
    // disconnecting from the server!
    std::recursive_mutex _activeGameMutex;
    std::optional<game_event_watcher::CsgoMatchState> _matchState;
    shared::TimePoint _matchStart;

    std::optional<std::string> _serverIdentifier;
    std::optional<std::string> _activeViewUuid;

    // Demos don't seem to be available as soon as the match is done. It seems more reasonable
    // to wait for the user to go back to the main menu before sending out the demo request.
    // Thus, when the game ends, we fill out the demo request. The demo request is fulfilled
    // only when exiting to the main menu or exiting the game.
    std::mutex _demoMutex;
    std::optional<DemoRequest> _demoRequest;
    void fulfillDemoRequest();
};

CsgoProcessHandlerInstance::CsgoProcessHandlerInstance(const process_watcher::process::Process& p):
    _logWatcher(std::make_unique<game_event_watcher::CsgoLogWatcher>(shared::nowUtc())),
    _process(p) {
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::CSGO);

    _stateManager.notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoGsiEvents::MatchStart), std::bind(&CsgoProcessHandlerInstance::onGsiMatchStart, this, std::placeholders::_1, std::placeholders::_2));
    _stateManager.notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoGsiEvents::MatchEnd), std::bind(&CsgoProcessHandlerInstance::onGsiMatchEnd, this, std::placeholders::_1, std::placeholders::_2));
    _stateManager.notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoGsiEvents::MainMenu), std::bind(&CsgoProcessHandlerInstance::onGsiMainMenu, this, std::placeholders::_1, std::placeholders::_2));

    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoLogEvents::Connect), std::bind(&CsgoProcessHandlerInstance::onLogConnect, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::ECsgoLogEvents::Disconnect), std::bind(&CsgoProcessHandlerInstance::onLogDisconnect, this, std::placeholders::_1, std::placeholders::_2));

    service::api::getGlobalApi()->markUserAnalyticsEvent("launch_game");
    service::api::getGlobalApi()->markUserAnalyticsEvent("launch_csgo");
}

CsgoProcessHandlerInstance::~CsgoProcessHandlerInstance() {
    fulfillDemoRequest();
}

void CsgoProcessHandlerInstance::forceStopRecording() {
    std::lock_guard guard(_activeGameMutex);
    _matchState.reset();
    if (_recorder->isRecording()) {
        _recorder->stop({});
    }

    _activeViewUuid.reset();
}

void CsgoProcessHandlerInstance::fulfillDemoRequest() {
    std::lock_guard guard(_demoMutex);
    if (!_demoRequest) {
        return;
    }

    const auto segmentId = shared::generateUuidv4();
    const auto strId = shared::generateUuidv4();

    shared::ipc::SharedMemory shmem(segmentId);
    shmem.create(65536);

    auto* demoUrl = shmem.createString(strId);

    bool foundDemo = false;

    const fs::path retrieverExe = shared::filesystem::getCurrentExeFolder() / fs::path("csgo") / fs::path("csgo_demo_retriever.exe");

    // Mike note: I'm a little worried of holding the _demoMutex lock for a whole minute but yolo?
    // Retry this a couple times just in case the demo isn't available immediately.
    for (int i = 0; i < 10; ++i) {
        // Run the csgo_demo_retriever app to pull in the latest demo that corresponds to the match the user just played.
        std::ostringstream cmd;
        cmd << " --mode steam "
            << " --threshold " << shared::timeToIso(_demoRequest.value().threshold)
            << " --shmem \"" << segmentId << "\""
            << " --shvalue \"" << strId << "\"";

        if (shared::process::runProcessWithTimeout(retrieverExe, cmd.str(), std::chrono::seconds(10))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            continue;
        }

        foundDemo = true;
        break;
    }

    if (foundDemo) {
        std::string rawDemoData(demoUrl->data(), demoUrl->size());
        std::vector<std::string> parts;
        boost::split(parts, rawDemoData, boost::is_any_of("."));

        const auto demoUrl = shared::base64::decode(parts[0]);
        const auto demoTimestamp = shared::isoStrToTime(shared::base64::decode(parts[1]));

        try {
            service::api::getGlobalApi()->associateCsgoDemo(_demoRequest.value().viewUuid, demoUrl, demoTimestamp);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to associate CSGO demo: " << ex.what() << std::endl);
        }
    }

    shmem.destroy<shared::ipc::SharedMemory::String>(strId);
    _demoRequest.reset();
}

void CsgoProcessHandlerInstance::onGsiMainMenu(const shared::TimePoint& eventTime, const void* rawData) {
    // Put this into a new thread so that we don't prevent ourselves from handling further GSI events.
    std::thread t([this](){
        fulfillDemoRequest();
    });
    t.detach();
}

void CsgoProcessHandlerInstance::onGsiMatchStart(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }
    LOG_INFO("CSGO On GSI Match Start..." << std::endl);

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
    LOG_INFO("CSGO On Log Connect..." << std::endl);

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
        LOG_INFO("..Ignoring due to no server identifier or match state." << std::endl);
        return;
    }

    if (_activeViewUuid) {
        LOG_WARNING("Trying to start a new CSGO match when one is already active?" << std::endl);
        return;
    }

    try {
        _activeViewUuid = service::api::getGlobalApi()->createNewCsgoMatch(_serverIdentifier.value(), eventTime, _matchState->map, _matchState->mode);
        LOG_INFO("Started new CSGO match @" << shared::timeToStr(eventTime) << std::endl
            << "\tServer: " << _serverIdentifier.value() << std::endl
            << "\tMap: " << _matchState->map << std::endl
            << "\tMode: " << _matchState->mode << std::endl
        );
        _matchStart = eventTime;
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

    LOG_INFO("CSGO On GSI Match End..." << std::endl);
    if (!_matchState) {
        LOG_WARNING("No match state...ignoring." << std::endl);
        return;
    }

    const auto matchStateCopy = _matchState.value();
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
        // Only CSGO competitive games get demos recorded.
        // This will probably need to be tweaked to support FaceIT and ESEA.
        if (matchStateCopy.mode == "competitive") {
            std::lock_guard guard(_demoMutex);
            DemoRequest req;

            // Subtract 10 minutes to account for a full fledged warmup because the
            // "match start" doesn't occur until after warmup occurs.
            req.threshold = _matchStart - std::chrono::minutes(10);
            req.viewUuid = _activeViewUuid.value();
            _demoRequest = req;
        }

        const auto matchUuid = service::api::getGlobalApi()->finishCsgoMatch(_activeViewUuid.value(), _stateManager.localSteamId(), eventTime, *state, std::nullopt, std::nullopt);
        end = std::make_optional(service::recorder::GameRecordEnd{
            matchUuid,
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
    LOG_INFO("CSGO On Log Disconnect..." << std::endl);

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

void CsgoProcessHandler::forceStopRecording() {
    if (!_instance) {
        return;
    }
    LOG_INFO("Force Stop Recording: CS:GO" << std::endl);
    _instance->forceStopRecording();
}

}