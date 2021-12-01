#include "system/settings.h"
#include "shared/filesystem/common_paths.h"
#include "shared/errors/error.h"
#include "shared/strings/strings.h"
#include "system/state.h"
#include <fstream>

namespace fs = std::filesystem;
namespace service::system {

Settings* getCurrentSettings() {
    static SettingsPtr global = std::make_unique<Settings>();
    return global.get();
}

AudioDeviceSettings AudioDeviceSettings::fromJson(const nlohmann::json& obj) {
    AudioDeviceSettings settings;
    settings.device = obj.value("device", "Default Device");
    settings.volume = obj.value("volume", 1.0);
    settings.mono = obj.value("mono", false);
    settings.voice = obj.value("voice", false);
    return settings;
}

RecordingSettings RecordingSettings::fromJson(const nlohmann::json& obj) {
    RecordingSettings settings;
    settings.resY = obj["resY"].get<int32_t>();
    settings.fps = obj["fps"].get<int32_t>();

    // TEMPORARY MEASURE FOR NVIDIA GPUS DUE TO #1260.
    settings.useVideoHw = obj.value("useVideoHw", true) && !service::system::getGlobalState()->isNvidiaGPU();

    settings.useHwEncoder = obj.value("useHwEncoder", true);

    if (obj.find("outputDevices") != obj.end() && obj.count("outputDevices") > 0) {
        for (const auto& o : obj["outputDevices"]) {
            settings.outputDevices.push_back(AudioDeviceSettings::fromJson(o));
        }
    }

    if (obj.find("inputDevices") != obj.end() && obj.count("inputDevices") > 0) {
        for (const auto& o : obj["inputDevices"]) {
            settings.inputDevices.push_back(AudioDeviceSettings::fromJson(o));
        }
    }

    settings.usePushToTalk = obj.value("usePushToTalk", false);
    settings.useVfr4 = obj.value("useVfr4", true);
    settings.useWGC2 = obj.value("useWGC2", true);

    settings.useLocalRecording = obj.value("useLocalRecording", false);
    settings.localRecordingLocation =  fs::path(shared::strings::utf8ToWcs(obj["localRecordingLocation"].get<std::string>()));
    settings.maxLocalRecordingSizeGb = obj["maxLocalRecordingSizeGb"].get<double>();

    settings.vodEndDelaySeconds = obj["vodEndDelaySeconds"].get<int32_t>();
    if (obj.count("overlays") > 0) {
        settings.overlays = service::renderer::OverlaySettings::fromJson(obj["overlays"]);
    }
    settings.useAudioDriftCompensation = obj.value("useAudioDriftCompensation", true);

    settings.useVoiceBasicNoiseFilter = obj.value("useVoiceBasicNoiseFilter", false);
    settings.voiceFilterThresholdDb = obj.value("voiceFilterThresholdDb", -60);
    settings.useVoiceSpeechNoiseReduction = obj.value("useVoiceSpeechNoiseReduction", false);
    return settings;
}

KeybindSettings KeybindSettings::fromJson(const nlohmann::json& obj) {
    KeybindSettings settings;
    for (const auto& ele: obj["pushToTalk"]) {
        settings.pushToTalk.push_back(ele.get<int>());
    }

    for (const auto& ele: obj["pushToTalk2"]) {
        settings.pushToTalk2.push_back(ele.get<int>());
    }
    return settings;
}

WowDisabledInstance WowDisabledInstance::fromJson(const nlohmann::json& obj) {
    WowDisabledInstance ret;
    ret.id = obj.value("id", -1);
    ret.release = static_cast<shared::EWowRelease>(obj.value("release", static_cast<int32_t>(shared::EWowRelease::Unknown)));
    return ret;
}

WowSettings WowSettings::fromJson(const nlohmann::json& obj) {
    WowSettings settings;
    settings.useCombatLogTimeout = obj.value("useCombatLogTimeout", true);
    settings.timeoutSeconds2 = obj.value("timeoutSeconds2", 180);
    settings.recordArenas = obj.value("recordArenas", true);
    settings.recordBattlegrounds = obj.value("recordBattlegrounds", true);
    settings.recordDungeons = obj.value("recordDungeons", true);
    settings.recordKeystones = obj.value("recordKeystones", true);
    settings.recordEncounters = obj.value("recordEncounters", true);
    settings.minimumTimeSecondsToRecord = obj.value("minimumTimeSecondsToRecord", 15);
    if (obj.find("doNotRecordInstances") != obj.end() && obj.count("doNotRecordInstances") > 0) {
        for (const auto& o : obj["doNotRecordInstances"]) {
            settings.doNotRecordInstances.push_back(WowDisabledInstance::fromJson(o));

            const auto& dnr = settings.doNotRecordInstances.back();
            settings.doNotRecordInstancesCached[dnr.release].insert(dnr.id);
        }
    }
    return settings;
}

ValorantSettings ValorantSettings::fromJson(const nlohmann::json& obj) {
    ValorantSettings settings;
    settings.recordStandard = obj.value("recordStandard", true);
    settings.recordSpikeRush = obj.value("recordSpikeRush", true);
    settings.recordDeathmatch = obj.value("recordDeathmatch", true);
    settings.recordOther = obj.value("recordOther", true);
    return settings;
}

PerGameSettings PerGameSettings::fromJson(const nlohmann::json& obj) {
    PerGameSettings settings;
    settings.wow = WowSettings::fromJson(obj["wow"]);

    if (obj.find("valorant") != obj.end()) {
        settings.valorant = ValorantSettings::fromJson(obj["valorant"]);
    }
    return settings;
}

LocalSettings LocalSettings::fromJson(const nlohmann::json& obj) {
    LocalSettings settings;
    settings.record = RecordingSettings::fromJson(obj["record"]);
    settings.keybinds = KeybindSettings::fromJson(obj["keybinds"]);
    settings.games = PerGameSettings::fromJson(obj["games"]);

    if (obj.count("disabledGames") > 0) {
        for (const auto& val : obj["disabledGames"]) {
            settings.disabledGames.insert(static_cast<shared::EGame>(val.get<int32_t>()));
        }
    }

    return settings;
}

Settings::Settings() {
    reloadSettingsFromFile();
}

void Settings::reloadSettingsFromFile() {
    std::unique_lock lock(_mutex);
    const auto fpath = shared::filesystem::getSquadOvUserSettingsFile();
    LOG_INFO("Loading Settings: " << fpath << std::endl);
    try {
        std::ifstream ifs(fpath);
        const auto obj = nlohmann::json::parse(ifs);
        _settings = LocalSettings::fromJson(obj);
        _loaded = true;
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to read settings (assuming default): " << ex.what() << std::endl);
    }
}

RecordingSettings Settings::recording() {
    std::shared_lock lock(_mutex);
    return _settings.record;
}

KeybindSettings Settings::keybinds() {
    std::shared_lock lock(_mutex);
    return _settings.keybinds;
}

WowSettings Settings::wowSettings() {
    std::shared_lock lock(_mutex);
    return _settings.games.wow;
}

ValorantSettings Settings::valorantSettings() {
    std::shared_lock lock(_mutex);
    return _settings.games.valorant;
}

bool Settings::isGameEnabled(shared::EGame game) {
    std::shared_lock lock(_mutex);
    return (_settings.disabledGames.find(game) == _settings.disabledGames.end());
}

}