#include "system/settings.h"
#include "shared/filesystem/common_paths.h"
#include <fstream>

namespace service::system {

Settings* getCurrentSettings() {
    static SettingsPtr global = std::make_unique<Settings>();
    return global.get();
}

RecordingSettings RecordingSettings::fromJson(const nlohmann::json& obj) {
    RecordingSettings settings;
    settings.resY = obj["resY"].get<int32_t>();
    settings.fps = obj["fps"].get<int32_t>();
    settings.useVideoHw = obj.value("useVideoHw", true);
    settings.outputDevice = obj["outputDevice"].get<std::string>();
    settings.outputVolume = obj["outputVolume"].get<double>();
    settings.inputDevice = obj["inputDevice"].get<std::string>();
    settings.inputVolume = obj["inputVolume"].get<double>();
    return settings;
}

LocalSettings LocalSettings::fromJson(const nlohmann::json& obj) {
    LocalSettings settings;
    settings.record = RecordingSettings::fromJson(obj["record"]);
    return settings;
}

Settings::Settings() {
    reloadSettingsFromFile();
}

void Settings::reloadSettingsFromFile() {
    std::ifstream ifs(shared::filesystem::getSquadOvUserSettingsFile());
    const auto obj = nlohmann::json::parse(ifs);
    _settings = LocalSettings::fromJson(obj);
}

}