#pragma once

#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_set>
#include <vector>

#include "shared/games.h"
#include "recorder/compositor/layers/overlay_layers.h"
#include "recorder/video/video_recorder.h"
#include "process_watcher/process/process.h"
#include "shared/json.h"
#include "system/keybinds.h"

namespace service::system {

struct AudioDeviceSettings {
    std::string device;
    std::string id;
    double volume = 1.0;
    bool mono = false;
    bool voice = false;

    static AudioDeviceSettings fromJson(const nlohmann::json& obj);
    AudioDeviceSettings createDefault() const;
};

struct ProcessAudioRecordSettings {
    process_watcher::process::ProcessRecord process;
    double volume = 1.0;

    static ProcessAudioRecordSettings fromJson(const nlohmann::json& obj);
};


enum class SquadOvPositionX {
    Left,
    Center,
    Right,
};

enum class SquadOvPositionY {
    Top,
    Center,
    Bottom,
};

struct WatermarkSettings {
    bool enabled;
    double size;
    SquadOvPositionX xPos;
    SquadOvPositionY yPos;

    static WatermarkSettings fromJson(const nlohmann::json& obj);
};

struct RecordingSettings {
    int32_t resY = 0;
    int32_t fps = 0;
    int32_t bitrateKbps = 0;
    bool useVideoHw2 = false;
    bool useHwEncoder = false;
    bool useVfr4 = true;
    bool useWGC2 = true;
    bool recordMouse2 = false;

    std::vector<AudioDeviceSettings> outputDevices;
    std::vector<AudioDeviceSettings> inputDevices;
    bool usePushToTalk = false;
    bool useWASAPIRecording3 = true;

    bool usePerProcessRecording = false;
    bool perProcessRecordingOsCheck = false;
    bool recordGameAudio = true;
    double gameAudioVolume = 1.0;
    std::vector<ProcessAudioRecordSettings> processesToRecord;

    bool useLocalRecording = false;
    std::filesystem::path localRecordingLocation;
    double maxLocalRecordingSizeGb = 1.0;

    int32_t vodEndDelaySeconds = 0;
    service::recorder::compositor::layers::OverlaySettings overlays;
    bool useAudioDriftCompensation = false;

    bool useVoiceBasicNoiseFilter = false;
    int32_t voiceFilterThresholdDb = -60;
    bool useVoiceSpeechNoiseReduction = false;
    
    bool useCbr = false;
    bool useNativeAspectRatio = false;
    std::optional<int32_t> bandwidthLimiterMultiple;
    WatermarkSettings watermark;
    service::recorder::video::VideoCodec videoCodec;

    static RecordingSettings fromJson(const nlohmann::json& obj);
};

struct KeybindSettings {
    service::system::Keybind pushToTalk;
    service::system::Keybind pushToTalk2;
    service::system::Keybind bookmark;
    service::system::Keybind clip;
    static KeybindSettings fromJson(const nlohmann::json& obj);
};

struct WowDisabledInstance {
    int64_t id;
    shared::EWowRelease release;

    static WowDisabledInstance fromJson(const nlohmann::json& obj);
};

struct WowSettings {
    bool useCombatLogTimeout = true;
    int32_t timeoutSeconds2 = 180;
    bool recordArenas = true;
    bool recordBattlegrounds = true;
    bool recordDungeons = true;
    bool recordKeystones = true;
    bool recordEncounters = true;
    bool recordScenarios = true;
    int32_t minimumTimeSecondsToRecord = 15;
    bool showAddonWarnings = true;

    // This is stored twice: doNotRecordInstances is the raw data stored in the settings JSON.
    // We also need a better way to query it: first by wow release and second by instance id to see
    // if it exists in the raw vector.
    std::vector<WowDisabledInstance> doNotRecordInstances;
    std::unordered_map<shared::EWowRelease, std::unordered_set<int64_t>> doNotRecordInstancesCached;

    static WowSettings fromJson(const nlohmann::json& obj);
};

struct ValorantSettings {
    bool recordStandard = true;
    bool recordSpikeRush = true;
    bool recordDeathmatch = true;
    bool recordOther = true;

    static ValorantSettings fromJson(const nlohmann::json& obj);
};

struct PerGameSettings {
    WowSettings wow;
    ValorantSettings valorant;

    static PerGameSettings fromJson(const nlohmann::json& obj);
};

struct LocalSettings {
    RecordingSettings record;
    KeybindSettings keybinds;
    PerGameSettings games;
    double speedCheckResultMbps = 0.0;
    std::unordered_set<shared::EGame> disabledGames;
    int64_t instantClipLengthSeconds = 15;

    static LocalSettings fromJson(const nlohmann::json& obj);
};

class Settings {
public:
    Settings();

    void reloadSettingsFromFile();

    double speedCheckResultMbps() const;
    int64_t instantClipLengthSeconds() const;

    RecordingSettings recording() const;
    KeybindSettings keybinds() const;

    WowSettings wowSettings() const;
    ValorantSettings valorantSettings() const;

    bool isGameEnabled(shared::EGame) const;
    
    bool loaded() const { return _loaded; }
    nlohmann::json raw() const { return _raw; }

private:
    mutable std::shared_mutex _mutex;
    LocalSettings _settings;

    // Need to store a raw version because we aren't necessarily going to be
    // loading up and parsing the entire json file so we need to make sure that
    // we store the OG so we can still save changes to disk.
    nlohmann::json _raw;
    bool _loaded = false;
};

using SettingsPtr = std::unique_ptr<Settings>;

Settings* getCurrentSettings();

}


namespace shared::json {

template<>
struct JsonConverter<service::system::AudioDeviceSettings> {
    static nlohmann::json to(const service::system::AudioDeviceSettings& v) {
        nlohmann::json data = {
            { "device", v.device },
            { "volume", v.volume },
            { "mono", v.mono },
            { "voice", v.voice },
            { "id", v.id }
        };
        return data;
    }
};

}