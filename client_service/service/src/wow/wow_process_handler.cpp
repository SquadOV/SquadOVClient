#include "wow/wow_process_handler.h"
#include "shared/log/log.h"
#include "game_event_watcher/wow/wow_log_watcher.h"
#include "api/squadov_api.h"
#include "shared/time.h"
#include "recorder/game_recorder.h"
#include "system/state.h"
#include "shared/http/http_client.h"
#include "shared/version.h"
#include "shared/env.h"
#include "shared/wow/instances.h"

#include <atomic>
#include <unordered_map>
#include <unordered_set>

#define TEST_DUMP 0

namespace service::wow {

class WoWProcessHandlerInstance {
public:
    WoWProcessHandlerInstance(const process_watcher::process::Process& p);
    ~WoWProcessHandlerInstance();

    void detectGameFromProcess();
    void cleanup();
    void overrideCombatLogPosition(const std::filesystem::path& path);
    void manualVodOverride(const std::filesystem::path& path, const shared::TimePoint& startTime);
    void waitForLogWatcher();
    void onProcessChange(const process_watcher::process::Process& p);
    void forceStopRecording();

    shared::EGame game() const { return _finalGame; }

    bool inMatch() const { return hasMatchView() && (inChallenge() || inEncounter() || inArena() || _currentInstance); }
    bool hasMatchView() const { return !_currentMatchViewUuid.empty(); }

private:
    process_watcher::process::Process  _process;
    game_event_watcher::WoWLogWatcherPtr _logWatcher;

    bool hasValidCombatLog() const;
    bool inChallenge() const { return !_currentChallenge.challengeName.empty(); }
    bool inEncounter() const { return !_currentEncounter.encounterName.empty(); }
    bool inArena() const { return !_currentArena.type.empty(); }
    bool checkIfInstanceIdShouldBeRecorded(int64_t id) const;

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
    void onSpellCastSuccess(const shared::TimePoint& tm, const void* data);

    void onInstanceStart(const shared::TimePoint& tm, const shared::wow::TypedInstanceData& data);
    void onInstanceEnd(const shared::TimePoint& tm);

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

    void forceKillLogTimeoutThread(bool force = false);

    std::recursive_mutex _currentMatchMutex;
    std::string _currentMatchViewUuid;
    game_event_watcher::WoWChallengeModeStart _currentChallenge;
    game_event_watcher::WoWEncounterStart _currentEncounter;
    game_event_watcher::WoWArenaStart _currentArena;
    std::optional<shared::wow::TypedInstanceData> _currentInstance;

    std::vector<game_event_watcher::WoWCombatantInfo> _combatants;
    bool _expectingCombatants = false;

    std::unordered_set<std::string> _instancePlayers;

    service::recorder::GameRecorderPtr _recorder;

    //
    std::filesystem::path _manualVodPath;
    shared::TimePoint _manualVodStartTime;

    std::unordered_map<int, shared::wow::TypedInstanceData> _instanceIdToData;

    shared::EGame _finalGame = shared::EGame::Unknown;
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
    detectGameFromProcess();

    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, _finalGame);
    if (!_process.empty()) {
        _recorder->startDvrSession();
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
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::SpellCastSuccess), std::bind(&WoWProcessHandlerInstance::onSpellCastSuccess, this, std::placeholders::_1, std::placeholders::_2));

    if (!_process.empty()) {
        _logWatcher->loadFromExecutable(p.path());
    } else {
        LOG_WARNING("Empty process. May or may not be an error?" << std::endl);
    }
}

WoWProcessHandlerInstance::~WoWProcessHandlerInstance() {
    if (_lastLogTimeoutThread.joinable()) {
        _lastLogTimeoutThread.join();
    }
}

void WoWProcessHandlerInstance::forceStopRecording() {
    prematurelyEndMatch(shared::nowUtc(), true, true);
}

bool WoWProcessHandlerInstance::checkIfInstanceIdShouldBeRecorded(int64_t id) const {
    const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
    const auto release = shared::gameToWowRelease(_finalGame);

    const auto releaseIt = wowSettings.doNotRecordInstancesCached.find(release);
    if (releaseIt == wowSettings.doNotRecordInstancesCached.end()) {
        return true;
    }

    const auto instanceIt = releaseIt->second.find(id);
    if (instanceIt == releaseIt->second.end()) {
        return true;
    }

    return false;
}

void WoWProcessHandlerInstance::detectGameFromProcess() {
    // If there is no process then we're probably in a test/dev environment and we can pull the data from an env variable instead for convenience.
    if (_process.empty()) {
        const auto env = shared::getEnv("SQUADOV_WOW_RELEASE");
        if (env == "vanilla") {
            _finalGame = shared::EGame::WowVanilla;
        } else if (env == "tbc") {
            _finalGame = shared::EGame::WowTbc;
        } else {
            _finalGame = shared::EGame::WoW;
        }
        return;
    }

    if (_process.name() == L"Wow.exe") {
        LOG_INFO("Detected WoW Retail..." << std::endl);
        _finalGame = shared::EGame::WoW;
    } else if (_process.name() == L"WowClassic.exe") {
        DWORD unk1 = 0;
        const auto fileInfoSize = GetFileVersionInfoSizeExW(0, _process.path().native().c_str(), &unk1);

        std::vector<char> buffer(fileInfoSize);
        if (!GetFileVersionInfoExW(0, _process.path().native().c_str(), 0, fileInfoSize, (void*)buffer.data())) {
            THROW_ERROR("Failed to get WoW EXE info: " << shared::errors::getWin32ErrorAsString());
        }

        char* infBuffer = nullptr;
        unsigned int infSize = 0;
        if (!VerQueryValueA((void*)buffer.data(), "\\StringFileInfo\\000004B0\\FileVersion", (void**)&infBuffer, &infSize)) {
            THROW_ERROR("Failed to query exe info.");
        }

        std::string versionString(infBuffer, infSize);
        const auto currentVersion = shared::parseVersionInfo(versionString);

        // 1.X.X is Vanilla and 2.X.X is TBC.
        if (currentVersion.major == 1) {
            _finalGame = shared::EGame::WowVanilla;
        } else if (currentVersion.major == 2) {
            _finalGame = shared::EGame::WowTbc;
        } else {
            THROW_ERROR("Unknown WoW classic version: " << currentVersion << std::endl);
        }
    } else {
        THROW_ERROR("Unknown WoW executable name: " << _process.path() << std::endl);
    }
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

void WoWProcessHandlerInstance::onProcessChange(const process_watcher::process::Process& p) {
    LOG_INFO("Change WoW Process from " << _process.pid() << " to " << p.pid() << "...Ending Matches" << std::endl);

    // Before we end the match, make sure the recorder gets pointed to the new process as well.
    if (!_recorder) {
        LOG_WARNING("...No recorder on WoW process change...?" << std::endl);
        return;
    }

    _recorder->setNewProcess(p);

    if (inMatch()) {
        // We want to end arenas, encounters, and challenges (hence true true).
        prematurelyEndMatch(shared::nowUtc(), true, true);
    } else {
        // And even if we aren't already in a match - we want to make sure we restart the DVR session
        // to pick up the new process (make sure we're recording the right window and what not).
        _recorder->stop({}, false);

        // Restart DVR!
        _recorder->startDvrSession();
    }
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

void WoWProcessHandlerInstance::forceKillLogTimeoutThread(bool force) {
    if (!_lastLogTimeoutRunning && !force) {
        return;
    }

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
    LOG_INFO("Using WoW combat log timeout of " << wowSettings.timeoutSeconds2 << " seconds..." << std::endl);
    
    while (_lastLogTimeoutRunning) {
        {
            const auto elapsedSeconds = [this](){
                std::lock_guard guard(_lastLogMutex);
                const auto elapsed = shared::nowUtc() - _lastLogTime;
                return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
            }();

            if (elapsedSeconds > wowSettings.timeoutSeconds2) {
                LOG_INFO("!!! WoW Combat Log Timeout !!!" << std::endl);
                _lastLogTimeoutRunning = false;
                prematurelyEndMatch(shared::nowUtc(), false, true);
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    LOG_INFO("...Exiting out of Log Timeout Thread." << std::endl);
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

    const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
    if (!wowSettings.recordEncounters) {
        LOG_INFO("...WoW Encounter Recording Disabled." << std::endl);
        return;
    }

    const auto tmpData = *reinterpret_cast<const game_event_watcher::WoWEncounterStart*>(data);
    if (!checkIfInstanceIdShouldBeRecorded(tmpData.instanceId)) {
        LOG_INFO("...WoW Instance Recording Disabled: " << tmpData.instanceId << std::endl);
        return;
    }

    _currentEncounter = tmpData;
    _matchStartTime = tm;
    LOG_INFO("WoW Encounter Start [" <<  shared::timeToStr(tm) << "]: " << _currentEncounter << std::endl);
    _expectingCombatants = true;
    genericMatchStart(tm);
}

void WoWProcessHandlerInstance::onEncounterEnd(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);
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

    if (!hasValidCombatLog()) {
        return;
    }

    const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
    if (!wowSettings.recordKeystones) {
        LOG_INFO("...WoW Challenge Recording Disabled." << std::endl);
        return;
    }

    const auto tmpData = *reinterpret_cast<const game_event_watcher::WoWChallengeModeStart*>(data);
    if (!checkIfInstanceIdShouldBeRecorded(tmpData.instanceId)) {
        LOG_INFO("...WoW Instance Recording Disabled: " << tmpData.instanceId << std::endl);
        return;
    }

    _currentChallenge = tmpData;
    LOG_INFO("WoW Challenge Start [" <<  shared::timeToStr(tm) << "]: " << _currentChallenge << std::endl);
    _expectingCombatants = true;
    if (inMatch() && _currentInstance) {
        // In this case we want to convert the instance view into a keystone view.
        // Functionally equivalent on the recording side.
        try {
            service::api::getGlobalApi()->convertWowInstanceViewToKeystone(_currentMatchViewUuid, tm, _currentChallenge, _combatLog);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to convert instance to WoW challenge: " << ex.what() << "\t" << _currentMatchViewUuid << std::endl);
        }
        _currentInstance = std::nullopt;
    } else if (!inMatch()) {
        _matchStartTime = tm;
        genericMatchStart(tm);
    } else {
        _currentChallenge = {};
        _expectingCombatants = false;
    }
}

void WoWProcessHandlerInstance::onChallengeModeEnd(const shared::TimePoint& tm, const void* data) {
    std::lock_guard guard(_currentMatchMutex);

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

    const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
    if (!wowSettings.recordArenas) {
        LOG_INFO("...WoW Arenas Recording Disabled." << std::endl);
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
    if (_currentInstance && (isZoneChange || isTimeout)) {
        onInstanceEnd(tm);
    } else if (inArena()) {
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
    bool canMatchEnd = hasValidCombatLog() && (inArena() || inChallenge() || inEncounter() || _currentInstance);

    if (!data) {
        return;
    }

    const auto zoneData = *reinterpret_cast<const game_event_watcher::WoWZoneChange*>(data);

    // Pre-emptively create the client that we'll use to grab additional information about the instance.
    // Note that we won't necessarily need to use it if this data is cached for example.
    std::ostringstream path;
    path << "/wow/"; 

    // We shouldn't ever need the final WoW condition unless the detect WoW game from process function failed at construction.
    if (game() == shared::EGame::WoW) {
        path << "9.1.5";
    } else if (game() == shared::EGame::WowVanilla) {
        path << "1.13.7";
    } else if (game() == shared::EGame::WowTbc) {
        path << "2.5.2";
    }

    path << "/instances/" << zoneData.instanceId << "/data.json";

    shared::http::HttpClient client("https://us-central1.content.squadov.gg");
    client.setTimeout(30);

    if (_instanceIdToData.find(zoneData.instanceId) == _instanceIdToData.end()) {
        try {
            auto resp = client.get(path.str());

            const auto parsedJson = nlohmann::json::parse(resp->body);
            const auto instanceData = shared::json::JsonConverter<shared::wow::InstanceData>::from(parsedJson).toTyped();
            _instanceIdToData[zoneData.instanceId] = instanceData;
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to send HTTP request to check instance type: " << ex.what() << std::endl);
        }
    }

    const auto it = _instanceIdToData.find(zoneData.instanceId);
    if (canMatchEnd) {
        // As long as we instance somewhere into something that we aren't generally recording
        // that'll probably be fine to determine if we want to end recording. I'm assuming
        // it's impossible to go from a party dungeon to a raid dungeon for example.
        const auto isMatchEnder = (it == _instanceIdToData.end()) ||
            (it->second.instanceType == shared::wow::InstanceType::NotInstanced) ||
            (it->second.instanceType == shared::wow::InstanceType::Unknown);
        LOG_INFO("Using Zone Change as Match End: " << isMatchEnder << std::endl);
        prematurelyEndMatch(tm, isMatchEnder, false);
    } else if (hasValidCombatLog() && !_currentInstance) {
        if (it == _instanceIdToData.end()) {
            LOG_WARNING("...Failed to find data for zone: " << zoneData.instanceId << " [" << zoneData.zoneName << "]" << std::endl);
            return;
        }

        const auto instanceType = it->second.instanceType;
        // Did we zone into an instance that we might want to record??
        // There's a couple of instance types that we care about:
        //  1) Arenas.
        //  2) RBGs.
        //  3) Dungeons.
        //  4) Raids.
        // Note that we're going to be generally relying on the hope that the combat log
        // won't ever timeout in the middle of running any of this content and the only
        // possible timeout will happen AFTER the user zones out.
        //
        // In the case of raids, we want to give users the option to either record the entire
        // instance or just singular encounters because they might want to review boss fights
        // immediately afterwards rather than waiting for the raid to finish.
        //
        // In the case of dungeons, generally we expect users to run the whole thing through so
        // this can be on by default. Note that if we detect that we're actually running a keystone
        // then we'll need to convert the instance to a keystone run.
        //
        // In the case of arenas, this will only be necessary in WoW classic where the ARENA_MATCH_START/STOP
        // markers in the combat log do not exist.
        //
        // There's nothing special of note for RBGs.
        const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
        if (
            (instanceType == shared::wow::InstanceType::PartyDungeon && wowSettings.recordDungeons) ||
            ((instanceType == shared::wow::InstanceType::ArenaBattlefield) && shared::isWowClassic(_finalGame) && wowSettings.recordArenas) ||
            (instanceType == shared::wow::InstanceType::PVPBattlefield && wowSettings.recordBattlegrounds) ||
            (instanceType == shared::wow::InstanceType::Scenario && wowSettings.recordScenarios)
        ) {
            onInstanceStart(tm, it->second);
        }
    }
}

void WoWProcessHandlerInstance::onInstanceStart(const shared::TimePoint& tm, const shared::wow::TypedInstanceData& data) {
    // Don't do a lock on the _currentMatchMutex because the only place
    // this gets called is in zone change which already locks that mutex.
    if (service::system::getGlobalState()->isPaused() || _currentInstance) {
        return;
    }

    if (!checkIfInstanceIdShouldBeRecorded(data.id)) {
        LOG_INFO("...WoW Instance Recording Disabled: " << data.id << std::endl);
        return;
    }

    _currentInstance = data;
    _instancePlayers.clear();
    LOG_INFO("WoW Instance Start [" <<  shared::timeToStr(tm) << "]: " << _currentInstance.value() << std::endl);;
    _matchStartTime = tm;
    genericMatchStart(tm);
}

void WoWProcessHandlerInstance::onInstanceEnd(const shared::TimePoint& tm) {
    std::lock_guard guard(_currentMatchMutex);
    if (!_currentInstance) {
        return;
    }

    LOG_INFO("WoW Instance End [" <<  shared::timeToStr(tm) << "]: " << _currentInstance.value() << std::endl);

    std::string matchUuid;

    // Do a sanity check - if we just completed a super short instance then just delete it
    // because there's no reason to create a full fledged match since it's probably just something
    // the user did in passing.
    const auto wowSettings = service::system::getCurrentSettings()->wowSettings();
    const auto totalMatchTime = std::chrono::duration_cast<std::chrono::milliseconds>(tm - _matchStartTime).count();
    if (!_currentMatchViewUuid.empty() && ((totalMatchTime / 1000) > wowSettings.minimumTimeSecondsToRecord)) {
        std::vector<game_event_watcher::WoWCombatantInfo> combatants;
        combatants.reserve(_instancePlayers.size());

        for (const auto& guid: _instancePlayers) {
            combatants.push_back(game_event_watcher::WoWCombatantInfo{
                guid
            });
        }

        try {
            matchUuid = service::api::getGlobalApi()->finishWowInstanceMatch(_currentMatchViewUuid, tm, combatants);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to finish WoW instance: " << ex.what() << "\t" << _currentMatchViewUuid << std::endl);
            matchUuid.clear();
        }

        _instancePlayers.clear();
    } else {
        LOG_WARNING("\tNo match UUID for instance end or time too short - ignoring match: " << _currentMatchViewUuid << "\t" << totalMatchTime << "ms" << std::endl);
    }

    _currentInstance = std::nullopt;
    genericMatchEnd(matchUuid, tm);
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

void WoWProcessHandlerInstance::onSpellCastSuccess(const shared::TimePoint& tm, const void* data) {
    if (!_currentInstance) {
        return;
    }

    const auto cast = reinterpret_cast<const game_event_watcher::WowSpellCastSuccess*>(data);
    // 0x500 = 0x400 (object type player) + 0x100 (player control)
    if ((cast->src.guid.rfind("Player-") == 0) && (cast->src.flags & 0x500)) {
        _instancePlayers.insert(cast->src.guid);
    }
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
        _recorder->start(tm, service::recorder::RecordingMode::DVR);
    } else {
        _recorder->startFromSource(_manualVodPath, _manualVodStartTime, tm);
    }

    // Use the current challenge/encounter data + combatant info to request a unique match UUID.
    try {
        if (_currentInstance) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWowInstanceMatch(_matchStartTime, _currentInstance.value(), _combatLog);
        } else if (inChallenge()) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWoWChallengeMatch(_matchStartTime, _currentChallenge, _combatLog);
        } else if (inEncounter()) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWoWEncounterMatch(_matchStartTime, _currentEncounter, _combatLog);
        } else if (inArena()) {
            _currentMatchViewUuid = service::api::getGlobalApi()->createWoWArenaMatch(_matchStartTime, _currentArena, _combatLog);
        } else {
            THROW_ERROR("Match start without challenge or encounter or arena or instance." << std::endl);
        }
        LOG_INFO("\tWoW Match View Uuid: " << _currentMatchViewUuid << std::endl);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create WoW challenge/encounter/arena/instance: " << ex.what() << std::endl
            << "\tChallenge: " << _currentChallenge << std::endl
            << "\tEncounter: " << _currentEncounter << std::endl
            << "\tArena: " << _currentArena << std::endl
            << "\tCombatants: " << _combatants << std::endl);
        if (_currentInstance) {
            LOG_WARNING("\tInstance: " << _currentInstance.value() << std::endl);
        }
        return;
    }

    markCombatLogActive();

    forceKillLogTimeoutThread(true);
    LOG_INFO("Start Combat Log Timeout" << std::endl);
    _lastLogTimeoutRunning = true;
    _lastLogTimeoutThread = std::thread(std::bind(&WoWProcessHandlerInstance::logTimeoutHandler, this));
}

void WoWProcessHandlerInstance::genericMatchEnd(const std::string& matchUuid, const shared::TimePoint& tm) {
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

            _recorder->startDvrSession();
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
    _instance = std::make_unique<WoWProcessHandlerInstance>(p);
    service::system::getGlobalState()->markGameRunning(_instance->game(), true);
}

void WoWProcessHandler::manualStartLogWatching(const std::filesystem::path& path, const std::filesystem::path& vodPath, const shared::TimePoint& vodStartTime) {
    onProcessStarts(process_watcher::process::Process{});
    _instance->manualVodOverride(vodPath, vodStartTime);
    _instance->overrideCombatLogPosition(path);
    _instance->waitForLogWatcher();

    // Wait for match to finish (so we can test the timeout functionality too).
    auto inGameCheck = std::thread([this](){
        while(_instance->hasMatchView()) {
            LOG_INFO("...Checking for in match..." << std::endl);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    inGameCheck.join();

    onProcessStops();
}

void WoWProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP WOW" << std::endl);
    service::system::getGlobalState()->markGameRunning(_instance->game(), false);
    _instance->cleanup();
    _instance.reset(nullptr);
    LOG_INFO("\tWoW fully stopped." << std::endl);
}

bool WoWProcessHandler::onProcessChange(const process_watcher::process::Process& p) {
    if (!_instance) {
        return false;
    }

    _instance->onProcessChange(p);
    return true;
}

void WoWProcessHandler::forceStopRecording() {
    if (!_instance) {
        return;
    }
    LOG_INFO("Force Stop Recording: WOW" << std::endl);
    _instance->forceStopRecording();
}

}