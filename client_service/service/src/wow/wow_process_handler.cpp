#include "wow/wow_process_handler.h"
#include "shared/log/log.h"
#include "game_event_watcher/wow/wow_log_watcher.h"
#include "api/squadov_api.h"
#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "system/state.h"
#include "shared/http/http_client.h"

#include <atomic>
#include <VersionHelpers.h>
#include <unordered_map>

#define TEST_DUMP 0

namespace service::wow {
namespace {

int getWowRecordingFlags() {
    return service::recorder::FLAG_DXGI_RECORDING |
        (IsWindows10OrGreater() ? service::recorder::FLAG_WGC_RECORDING : service::recorder::FLAG_GDI_RECORDING);    
}

}

class WoWProcessHandlerInstance {
public:
    WoWProcessHandlerInstance(const process_watcher::process::Process& p);
    ~WoWProcessHandlerInstance();

    void cleanup();
    void overrideCombatLogPosition(const std::filesystem::path& path);
    void manualVodOverride(const std::filesystem::path& path, const shared::TimePoint& startTime);
    void waitForLogWatcher();

private:
    process_watcher::process::Process  _process;
    game_event_watcher::WoWLogWatcherPtr _logWatcher;

    bool hasValidCombatLog() const;
    bool inChallenge() const { return !_currentChallenge.challengeName.empty(); }
    bool inEncounter() const { return !_currentEncounter.encounterName.empty(); }
    bool inArena() const { return !_currentArena.type.empty(); }
    bool inMatch() const { return !_currentMatchViewUuid.empty() && (inChallenge() || inEncounter() || inArena()); }

    // The general story of how we expect these events to play out is as follows:
    // 1) The user starts the combat log via /combatlog and we obtain a COMBAT_LOG_VERSION
    //    log line (EWoWLogEvents::CombatLogStart). We create a unique identifier for all the log lines that follow. 
    // 1b) For every log line that comes after COMBAT_LOG_VERSION, we send it to the
    //     server for storage/processing (EWoWLogEvents::CombatLogLine).
    // 2) The user begins a raid/instance and either ENCOUNTER_START or CHALLENGE_MODE_START
    //    occurs in the log (EWoWLogEvents::EncounterStart/EWoWLogEvents::ChallengeModeStart).
    // 3) Immediately following ENCOUNTER_START or CHALLENGE_MODE_START are N COMBATANT_INFO lines
    //    (EWoWLogEvents::CombatantInfo).
    // 4) After we collect all the combatant, we can obtain a unique match UUID for this particular
    //    encounter/challenge that can be synced across the users in the encounter/challenge. It is
    //    at this point that we can start recording.
    // 5) Finally, when the encounter/challenge is finished, either ENCOUNTER_END or
    //    CHALLENGE_MODE_END is called (EWoWLogEvents::EncounterEnd/EWoWLogEvents::ChallengeModeEnd)
    //    at which point we can stop recording.
    //
    // As a special note, it is possible that an ENCOUNTER_START happens within a
    // CHALLENGE_MODE_START/CHALLENGE_MODE_END block. In that case, CHALLENGE_MODE_START
    // has priority and the ENCOUNTER_START is merely marked as an interesting event.
    // This, however, is handled by the server as the client is only responsible for doing
    // things that enable it to record VODs.
    void onCombatLogStart(const shared::TimePoint& tm, const void* data);
    void onCombatLogLine(const shared::TimePoint& tm, const void* data);
    void onEncounterStart(const shared::TimePoint& tm, const void* data);
    void onEncounterEnd(const shared::TimePoint& tm, const void* data);
    void onChallengeModeStart(const shared::TimePoint& tm, const void* data);
    void onChallengeModeEnd(const shared::TimePoint& tm, const void* data);
    void onArenaStart(const shared::TimePoint& tm, const void* data);
    void onArenaEnd(const shared::TimePoint& tm, const void* data);
    void onZoneChange(const shared::TimePoint& tm, const void* data);
    void onCombatantInfo(const shared::TimePoint& tm, const void* data);
    void onFinishCombatantInfo(const shared::TimePoint& tm, const void* data);

    void genericMatchStart(const shared::TimePoint& tm);
    void genericMatchEnd(const std::string& matchUuid, const shared::TimePoint& tm);
    void prematurelyEndMatch(const shared::TimePoint& tm, bool isZoneChange, bool isTimeout);

    bool _combatLogActive = false;
    game_event_watcher::WoWCombatLogState _combatLog;
    shared::TimePoint _matchStartTime;

    void logTimeoutHandler();
    void markCombatLogActive();

    std::mutex _lastLogMutex;
    shared::TimePoint _lastLogTime;
    std::thread _lastLogTimeoutThread;
    std::atomic<bool> _lastLogTimeoutRunning = false;

    void forceKillLogTimeoutThread();

    std::recursive_mutex _currentMatchMutex;
    std::string _currentMatchViewUuid;
    game_event_watcher::WoWChallengeModeStart _currentChallenge;
    game_event_watcher::WoWEncounterStart _currentEncounter;
    game_event_watcher::WoWArenaStart _currentArena;

    std::vector<game_event_watcher::WoWCombatantInfo> _combatants;
    bool _expectingCombatants = false;

    service::recorder::GameRecorderPtr _recorder;

    //
    std::filesystem::path _manualVodPath;
    shared::TimePoint _manualVodStartTime;

    // Set of known zones and whether zoning there should quality as a map change.
    std::unordered_map<int, bool> _instanceIdIsMatchEnd;

#ifndef NDEBUG
#if TEST_DUMP
    nlohmann::json _debugSendLog;
#endif
#endif
};

WoWProcessHandlerInstance::WoWProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p),
    _logWatcher(new game_event_watcher::WoWLogWatcher(true, shared::nowUtc())),
    _lastLogTime(shared::nowUtc())
{
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::WoW);
    if (!_process.empty()) {
        _recorder->startDvrSession(getWowRecordingFlags());
    }

    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::CombatLogStart), std::bind(&WoWProcessHandlerInstance::onCombatLogStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::CombatLogLine), std::bind(&WoWProcessHandlerInstance::onCombatLogLine, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::EncounterStart), std::bind(&WoWProcessHandlerInstance::onEncounterStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::EncounterEnd), std::bind(&WoWProcessHandlerInstance::onEncounterEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::ChallengeModeStart), std::bind(&WoWProcessHandlerInstance::onChallengeModeStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::ChallengeModeEnd), std::bind(&WoWProcessHandlerInstance::onChallengeModeEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::ArenaStart), std::bind(&WoWProcessHandlerInstance::onArenaStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::ArenaEnd), std::bind(&WoWProcessHandlerInstance::onArenaEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::ZoneChange), std::bind(&WoWProcessHandlerInstance::onZoneChange, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::CombatantInfo), std::bind(&WoWProcessHandlerInstance::onCombatantInfo, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::FinishCombatantInfo), std::bind(&WoWProcessHandlerInstance::onFinishCombatantInfo, this, std::placeholders::_1, std::placeholders::_2));

    if (!_process.empty()) {
        _logWatcher->loadFromExecutable(p.path());
    } else {
        LOG_WARNING("Empty process. May or may not be an error?" << std::endl);
    }
}

WoWProcessHandlerInstance::~WoWProcessHandlerInstance() {
    forceKillLogTimeoutThread();
}

void WoWProcessHandlerInstance::overrideCombatLogPosition(const std::filesystem::path& path) {
    _logWatcher->setUseTimeChecks(false);
    _logWatcher->loadFromPath(path, false);
}

void WoWProcessHandlerInstance::manualVodOverride(const std::filesystem::path& path, const shared::TimePoint& startTime) {
    _manualVodPath = path;
    _manualVodStartTime = startTime;
}

void WoWProcessHandlerInstance::waitForLogWatcher() {
    _logWatcher->wait();
}

void WoWProcessHandlerInstance::cleanup() {
    // Move the current combat log to backup so that it doesn't get too big.
    // This assumes that this is being called on WoW process stop so this should be safe to do.
    LOG_INFO("Ending DVR session..." << std::endl);
    _recorder->stop({});

    if (_logWatcher->legacy()) {
        LOG_INFO("Doing Legacy Log Cleanup..." << std::endl);
        _logWatcher->moveLogToBackup();
    }

    LOG_INFO("Finish WoW cleanup..." << std::endl);
}

bool WoWProcessHandlerInstance::hasValidCombatLog() const {
    return _combatLogActive;
}

void WoWProcessHandlerInstance::onCombatLogStart(const shared::TimePoint& tm, const void* data) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    const game_event_watcher::WoWCombatLogState* log = reinterpret_cast<const game_event_watcher::WoWCombatLogState*>(data);
    LOG_INFO("Start WoW Combat Log [Version " << log->combatLogVersion << "- Advanced " << log->advancedLog << " - Build " << log->buildVersion << "] @ " << shared::timeToStr(tm) << std::endl);

    try {
        service::api::getGlobalApi()->retrieveSessionFeatureFlags();
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to refresh session feature flags: " << ex.what() << std::endl);
    }

    _combatLogActive = true;
    _combatLog = *log;
}

void WoWProcessHandlerInstance::markCombatLogActive() {
    std::lock_guard guard(_lastLogMutex);
    _lastLogTime = shared::nowUtc();
}

void WoWProcessHandlerInstance::onCombatLogLine(const shared::TimePoint& tm, const void* data) {
    markCombatLogActive();

    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!hasValidCombatLog()) {
        return;
    }

    // Send combat log line to server associated with the current combat log.
    auto log = *reinterpret_cast<const game_event_watcher::RawWoWCombatLog*>(data);

    // The server doesn't need this so we don't send it to save on ingress/egress.
    log.log.clear();

    if (inMatch()) {
        const auto flags = service::api::getGlobalApi()->getSessionFeatures();
        if (flags.allowWowCombatLogUpload) {
            try {
                service::api::getKafkaApi()->uploadWoWCombatLogLine(_currentMatchViewUuid, log);

#ifndef NDEBUG
#if TEST_DUMP
                _debugSendLog.push_back(log.toJson());
#endif
#endif
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to upload combat log line: " << ex.what() << "\t" << _currentMatchViewUuid << std::endl);
            }
        }
    }
}

void WoWProcessHandlerInstance::forceKillLogTimeoutThread() {
    _lastLogTimeoutRunning = false;
    if (_lastLogTimeoutThread.joinable()) {
        _lastLogTimeoutThread.join();
    }
}

void WoWProcessHandlerInstance::logTimeoutHandler() {
    const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
    if (!wowSettings.useCombatLogTimeout) {
        return;
    }
    LOG_INFO("Using WoW combat log timeout of " << wowSettings.timeoutSeconds << " seconds..." << std::endl);
    
    while (_lastLogTimeoutRunning) {
        std::lock_guard guard(_lastLogMutex);
        const auto elapsed = shared::nowUtc() - _lastLogTime;
        const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

        if (elapsedSeconds > wowSettings.timeoutSeconds) {
            LOG_INFO("!!! WoW Combat Log Timeout !!!" << std::endl);
            prematurelyEndMatch(shared::nowUtc(), false, true);
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void WoWProcessHandlerInstance::onEncounterStart(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);

    if (inChallenge() && _combatants.empty()) {
        _expectingCombatants = true;
    }

    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!hasValidCombatLog() || inMatch()) {
        return;
    }

    _currentEncounter = *reinterpret_cast<const game_event_watcher::WoWEncounterStart*>(data);
    _matchStartTime = tm;
    LOG_INFO("WoW Encounter Start [" <<  shared::timeToStr(tm) << "]: " << _currentEncounter << std::endl);
    _expectingCombatants = true;
    genericMatchStart(tm);
}

void WoWProcessHandlerInstance::onEncounterEnd(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!hasValidCombatLog() || !inEncounter()) {
        return;
    }

    const auto end = *reinterpret_cast<const game_event_watcher::WoWEncounterEnd*>(data);
    LOG_INFO("WoW Encounter End [" <<  shared::timeToStr(tm) << "]: " << end << std::endl);
    std::string matchUuid;
    if (!_currentMatchViewUuid.empty()) {
        try {
            matchUuid = service::api::getGlobalApi()->finishWoWEncounterMatch(_currentMatchViewUuid, tm, end, _combatants);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to finish WoW Encounter: " << ex.what() << "\t" << _currentMatchViewUuid << std::endl);
            matchUuid.clear();
        }
    } else {
        LOG_WARNING("\tNo match UUID for encounter end?" << std::endl);
    }
    _currentEncounter = {};
    _combatants.clear();
    genericMatchEnd(matchUuid, tm);
}

void WoWProcessHandlerInstance::onChallengeModeStart(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!hasValidCombatLog() || inMatch()) {
        return;
    }

    _currentChallenge = *reinterpret_cast<const game_event_watcher::WoWChallengeModeStart*>(data);
    _matchStartTime = tm;
    LOG_INFO("WoW Challenge Start [" <<  shared::timeToStr(tm) << "]: " << _currentChallenge << std::endl);
    _expectingCombatants = true;
    genericMatchStart(tm);
}

void WoWProcessHandlerInstance::onChallengeModeEnd(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }
    
    if (!hasValidCombatLog() || !inChallenge()) {
        return;
    }

    const auto end = *reinterpret_cast<const game_event_watcher::WoWChallengeModeEnd*>(data);
    LOG_INFO("WoW Challenge End [" <<  shared::timeToStr(tm) << "]: " << end  << std::endl);

    std::string matchUuid;
    if (!_currentMatchViewUuid.empty()) {
        try {
            matchUuid = service::api::getGlobalApi()->finishWoWChallengeMatch(_currentMatchViewUuid, tm, end, _combatants);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to finish WoW challenge: " << ex.what() << "\t" << _currentMatchViewUuid << std::endl);
            matchUuid.clear();
        }
    } else {
        LOG_WARNING("\tNo match UUID?" << std::endl);
    }
    _currentChallenge = {};
    _combatants.clear();
    genericMatchEnd(matchUuid, tm);
}

void WoWProcessHandlerInstance::onArenaStart(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!hasValidCombatLog() || inMatch()) {
        return;
    }

    _currentArena = *reinterpret_cast<const game_event_watcher::WoWArenaStart*>(data);
    _matchStartTime = tm;
    LOG_INFO("WoW Arena Start [" <<  shared::timeToStr(tm) << "]: " << _currentArena << std::endl);
    _expectingCombatants = true;
    genericMatchStart(tm);
}

void WoWProcessHandlerInstance::onArenaEnd(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }
    
    if (!hasValidCombatLog() || !inArena()) {
        return;
    }

    const auto end = *reinterpret_cast<const game_event_watcher::WoWArenaEnd*>(data);
    LOG_INFO("WoW Arena End [" <<  shared::timeToStr(tm) << "]: " << end  << std::endl);

    std::string matchUuid;
    if (!_currentMatchViewUuid.empty()) {
        try {
            matchUuid = service::api::getGlobalApi()->finishWoWArenaMatch(_currentMatchViewUuid, tm, end, _combatants);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to finish WoW arena: " << ex.what() << "\t" << _currentMatchViewUuid << std::endl);
            matchUuid.clear();
        }
    } else {
        LOG_WARNING("\tNo match UUID?" << std::endl);
    }
    _currentArena = {};
    _combatants.clear();
    genericMatchEnd(matchUuid, tm);
}

void WoWProcessHandlerInstance::prematurelyEndMatch(const shared::TimePoint& tm, bool isZoneChange, bool isTimeout) {
    std::lock_guard guard(_currentMatchMutex);
    if (inArena()) {
        game_event_watcher::WoWArenaEnd end;
        end.winningTeamId = (_currentArena.localTeamId + 1) % 2;
        end.matchDurationSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(tm - _matchStartTime).count());
        end.newRatings[0] = 0;
        end.newRatings[1] = 0;
        onArenaEnd(tm, &end);
    } else if (inChallenge() && (isZoneChange || isTimeout)) {
        game_event_watcher::WoWChallengeModeEnd end;
        end.instanceId = _currentChallenge.instanceId;
        end.keystoneLevel = _currentChallenge.keystoneLevel;
        end.success = false;
        end.timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(tm - _matchStartTime).count();
        onChallengeModeEnd(tm, &end);
    } else if (inEncounter() && isTimeout) {
        // I'm not aware of any encounters that do a zoneout so no need to handle that.
        game_event_watcher::WoWEncounterEnd end;
        end.encounterId = _currentEncounter.encounterId;
        end.encounterName = _currentEncounter.encounterName;
        end.difficulty = _currentEncounter.difficulty;
        end.numPlayers = _currentEncounter.numPlayers;
        end.success = false;
        onEncounterEnd(tm, &end);
    }
}

void WoWProcessHandlerInstance::onZoneChange(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
    bool isMatchEnd = hasValidCombatLog() && (inArena() || inChallenge() || inEncounter());

    if (!isMatchEnd || !data) {
        return;
    }

    const auto zoneData = *reinterpret_cast<const game_event_watcher::WoWZoneChange*>(data);

    // We need to check that we're zoning out of the instance to qualify this as a match ender. Note that we 
    // assume that we only have instances/arenas on the server - maybe we should check instance type one day.
    if (_instanceIdIsMatchEnd.find(zoneData.instanceId) == _instanceIdIsMatchEnd.end()) {
        try {
            std::ostringstream path;
            path << "/wow/9.1.0/instances/" << zoneData.instanceId << "/data.json";

            shared::http::HttpClient client("https://us-central1.content.squadov.gg");
            client.setTimeout(30);

            auto resp = client.get(path.str());
            _instanceIdIsMatchEnd[zoneData.instanceId] = (resp->status != 200);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to send HTTP request to check zone info: " << ex.what() << std::endl);
            _instanceIdIsMatchEnd[zoneData.instanceId] = true;
        }
    }

    const auto isMatchEnder = _instanceIdIsMatchEnd[zoneData.instanceId];
    LOG_INFO("Using Zone Change as Match End: " << isMatchEnder << std::endl);
    prematurelyEndMatch(tm, isMatchEnder, false);
}

void WoWProcessHandlerInstance::onCombatantInfo(const shared::TimePoint& tm, const void* data) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!_expectingCombatants) {
        return;
    }

    const auto combatant = *reinterpret_cast<const game_event_watcher::WoWCombatantInfo*>(data);
    LOG_INFO("WoW Combatant Info [" <<  shared::timeToStr(tm) << "]: " << combatant << std::endl);
    _combatants.push_back(combatant);
}

void WoWProcessHandlerInstance::onFinishCombatantInfo(const shared::TimePoint& tm, const void* data) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!_expectingCombatants) {
        return;
    }

    LOG_INFO("WoW Finish Combatant Info [" << shared::timeToStr(tm) << "]" << std::endl);
    _expectingCombatants = false;
}

void WoWProcessHandlerInstance::genericMatchStart(const shared::TimePoint& tm) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!hasValidCombatLog()) {
        return;
    }

    LOG_INFO("WoW Match Start [" << shared::timeToStr(tm) << "::" << shared::timeToStr(_matchStartTime) << "] - VIEW " << _currentMatchViewUuid << std::endl);

    // Start recording first just in case the API takes a long time to respond.
    if (!_process.empty()) {
        _recorder->start(tm, service::recorder::RecordingMode::DVR, getWowRecordingFlags());
    } else {
        _recorder->startFromSource(_manualVodPath, _manualVodStartTime, tm);
    }

    // Use the current challenge/encounter data + combatant info to request a unique match UUID.
    try {
        if (inChallenge()) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWoWChallengeMatch(_matchStartTime, _currentChallenge, _combatLog);
        } else if (inEncounter()) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWoWEncounterMatch(_matchStartTime, _currentEncounter, _combatLog);
        } else if (inArena()) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWoWArenaMatch(_matchStartTime, _currentArena, _combatLog);
        } else {
            THROW_ERROR("Match start without challenge or encounter or arena." << std::endl);
        }
        LOG_INFO("\tWoW Match View Uuid: " << _currentMatchViewUuid << std::endl);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create WoW challenge/encounter/arena: " << ex.what() << std::endl
            << "\tChallenge: " << _currentChallenge << std::endl
            << "\tEncounter: " << _currentEncounter << std::endl
            << "\tArena: " << _currentArena << std::endl
            << "\tCombatants: " << _combatants << std::endl);
        return;
    }

    markCombatLogActive();

    forceKillLogTimeoutThread();
    LOG_INFO("Start Combat Log Timeout" << std::endl);
    _lastLogTimeoutThread = std::thread(std::bind(&WoWProcessHandlerInstance::logTimeoutHandler, this));
}

void WoWProcessHandlerInstance::genericMatchEnd(const std::string& matchUuid, const shared::TimePoint& tm) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    LOG_INFO("WoW Match End [" << shared::timeToStr(tm) << "] - VIEW " << _currentMatchViewUuid << std::endl);
    const auto isRecording = _recorder->isRecording();
    if (isRecording) {
        const auto vodId = _recorder->currentId();
        const auto sessionId = _recorder->sessionId();
        const auto vodStartTime = _recorder->vodStartTime();

        if (!_process.empty()) {
            if (!matchUuid.empty()) {
                service::recorder::GameRecordEnd end;
                end.matchUuid = matchUuid;
                end.endTime = tm;
                _recorder->stop(end);
            } else {
                _recorder->stop({});
            }

            _recorder->startDvrSession(getWowRecordingFlags());
        } else {
            service::recorder::GameRecordEnd end;
            end.matchUuid = matchUuid;
            end.endTime = tm;
            _recorder->stopFromSource(tm, end);
        }
    }

#ifndef NDEBUG
#if TEST_DUMP
    {
        std::ofstream outputFile("output.json");
        outputFile << _debugSendLog;
    }
#endif
#endif

    // Tell the server that the combat log is finished. I don't think this is *technically*
    // needed but a nice marker to have to force a flush out of events.
    const game_event_watcher::RawWoWCombatLog endLog = {
        shared::nowUtc(),
        { "SQUADOV_END_COMBAT_LOG" },
        "SQUADOV_END_COMBAT_LOG"
        -1
    };
    LOG_INFO("Sending SQUADOV_END_COMBAT_LOG " << hasValidCombatLog() << std::endl);
    onCombatLogLine(endLog.timestamp, (void*)&endLog);

    _currentMatchViewUuid = "";
    forceKillLogTimeoutThread();
}

WoWProcessHandler::WoWProcessHandler() = default;
WoWProcessHandler::~WoWProcessHandler() = default;

void WoWProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START WOW" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::WoW, true);
    _instance = std::make_unique<WoWProcessHandlerInstance>(p);
}

void WoWProcessHandler::manualStartLogWatching(const std::filesystem::path& path, const std::filesystem::path& vodPath, const shared::TimePoint& vodStartTime) {
    onProcessStarts(process_watcher::process::Process{});
    _instance->manualVodOverride(vodPath, vodStartTime);
    _instance->overrideCombatLogPosition(path);
    _instance->waitForLogWatcher();
    onProcessStops();
}

void WoWProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP WOW" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::WoW, false);
    _instance->cleanup();
    _instance.reset(nullptr);
    LOG_INFO("\tWoW fully stopped." << std::endl);
}

}