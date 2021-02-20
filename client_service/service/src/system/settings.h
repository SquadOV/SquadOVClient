#pragma once

#include <memory>
#include <nlohmann/json.hpp>

namespace service::system {

struct RecordingSettings {
    int32_t resY = 0;
    int32_t fps = 0;
    std::string outputDevice;
    double outputVolume;
    std::string inputDevice;
    double inputVolume;

    static RecordingSettings fromJson(const nlohmann::json& obj);
};

struct LocalSettings {
    RecordingSettings record;

    static LocalSettings fromJson(const nlohmann::json& obj);
};

class Settings {
public:
    Settings();

    void reloadSettingsFromFile();

    const RecordingSettings& recording() { return _settings.record; }

private:
    LocalSettings _settings;
};

using SettingsPtr = std::unique_ptr<Settings>;

Settings* getCurrentSettings();

}