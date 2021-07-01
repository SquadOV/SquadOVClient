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

    std::optional<size_t> maxUploadSpeed;

    bool useLocalRecording = false;
    std::filesystem::path localRecordingLocation;
    double maxLocalRecordingSizeGb = 1.0;

    bool useBitrate = false;
    int64_t bitrateKbps = 6000;

    int32_t vodEndDelaySeconds = 0;
    service::renderer::OverlaySettings overlays;

    static RecordingSettings fromJson(const nlohmann::json& obj);
};

struct KeybindSettings {
    std::vector<int> pushToTalk;
    static KeybindSettings fromJson(const nlohmann::json& obj);
};

struct LocalSettings {
    RecordingSettings record;
    KeybindSettings keybinds;
    bool enableNtp = true;
    std::unordered_set<shared::EGame> disabledGames;

    static LocalSettings fromJson(const nlohmann::json& obj);
};

class Settings {
public:
    Settings();

    void reloadSettingsFromFile();

    RecordingSettings recording();
    KeybindSettings keybinds();
    bool enableNtp();
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