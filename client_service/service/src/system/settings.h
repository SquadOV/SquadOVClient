#pragma once

#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_set>
#include <vector>

#include "shared/games.h"
#include "renderer/d3d11_overlay_renderer.h"

namespace service::system {

struct AudioDeviceSettings {
    std::string device;
    double volume = 1.0;
    bool mono = false;
    bool voice = false;

    static AudioDeviceSettings fromJson(const nlohmann::json& obj);
};

struct RecordingSettings {
    int32_t resY = 0;
    int32_t fps = 0;
    bool useVideoHw = false;
    bool useHwEncoder = false;
    bool useVfr3 = false;

    std::vector<AudioDeviceSettings> outputDevices;
    std::vector<AudioDeviceSettings> inputDevices;
    bool usePushToTalk = false;

    bool useLocalRecording = false;
    std::filesystem::path localRecordingLocation;
    double maxLocalRecordingSizeGb = 1.0;

    int32_t vodEndDelaySeconds = 0;
    service::renderer::OverlaySettings overlays;
    bool useAudioDriftCompensation = false;

    bool useVoiceBasicNoiseFilter = false;
    int32_t voiceFilterThresholdDb = -60;
    bool useVoiceSpeechNoiseReduction = false;

    static RecordingSettings fromJson(const nlohmann::json& obj);
};

struct KeybindSettings {
    std::vector<int> pushToTalk;
    static KeybindSettings fromJson(const nlohmann::json& obj);
};

struct WowSettings {
    bool useCombatLogTimeout = true;
    int32_t timeoutSeconds = 20;
    bool recordArenas = true;
    bool recordKeystones = true;
    bool recordEncounters = true;

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
    std::unordered_set<shared::EGame> disabledGames;
    bool enableDnsOverride = true;

    static LocalSettings fromJson(const nlohmann::json& obj);
};

class Settings {
public:
    Settings();

    void reloadSettingsFromFile();

    RecordingSettings recording();
    KeybindSettings keybinds();

    WowSettings wowSettings();
    ValorantSettings valorantSettings();

    bool enableDns();
    bool isGameEnabled(shared::EGame);
    
    bool loaded() const { return _loaded; }

private:
    std::shared_mutex _mutex;
    LocalSettings _settings;
    bool _loaded = false;
};

using SettingsPtr = std::unique_ptr<Settings>;

Settings* getCurrentSettings();

}