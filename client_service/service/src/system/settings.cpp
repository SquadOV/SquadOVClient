#include "system/settings.h"
#include "shared/filesystem/common_paths.h"
#include "shared/errors/error.h"
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
    settings.useHwEncoder = obj.value("useHwEncoder", true);
    settings.outputDevice = obj["outputDevice"].get<std::string>();
    settings.outputVolume = obj["outputVolume"].get<double>();
    settings.inputDevice = obj["inputDevice"].get<std::string>();
    settings.inputVolume = obj["inputVolume"].get<double>();
    settings.useVfr2 = obj.value("useVfr2", false);

    if (obj.count("maxUploadSpeed") != 0 && !obj["maxUploadSpeed"].is_null()) {
        settings.maxUploadSpeed = obj["maxUploadSpeed"].get<size_t>();
    } else {
        settings.maxUploadSpeed = {};
    }
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
    try {
        std::ifstream ifs(shared::filesystem::getSquadOvUserSettingsFile());
        const auto obj = nlohmann::json::parse(ifs);
        _settings = LocalSettings::fromJson(obj);
    } catch (std::exception& ex) {
        THROW_ERROR("Failed to read settings: " << ex.what() << std::endl);
    }
}

}