#pragma once

#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

namespace service::system {

struct RecordingSettings {
    int32_t resY = 0;
    int32_t fps = 0;
    bool useVideoHw = false;
    bool useHwEncoder = false;
    bool useVfr3 = false;

    std::string outputDevice;
    double outputVolume;
    std::string inputDevice;
    double inputVolume;
    bool usePushToTalk;

    std::optional<size_t> maxUploadSpeed;

    bool useLocalRecording;
    std::filesystem::path localRecordingLocation;
    double maxLocalRecordingSizeGb;

    bool useBitrate;
    int64_t bitrateKbps;

    static RecordingSettings fromJson(const nlohmann::json& obj);
};

struct KeybindSettings {
    std::vector<int> pushToTalk;
    static KeybindSettings fromJson(const nlohmann::json& obj);
};

struct LocalSettings {
    RecordingSettings record;
    KeybindSettings keybinds;

    static LocalSettings fromJson(const nlohmann::json& obj);
};

class Settings {
public:
    Settings();

    void reloadSettingsFromFile();

    RecordingSettings recording();
    KeybindSettings keybinds();
    
    bool loaded() const { return _loaded; }

private:
    std::shared_mutex _mutex;
    LocalSettings _settings;
    bool _loaded = false;
};

using SettingsPtr = std::unique_ptr<Settings>;

Settings* getCurrentSettings();

}