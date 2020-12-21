#include "shared/filesystem/common_paths.h"

#include "shared/errors/error.h"
#include "shared/env.h"

namespace fs = std::filesystem;
namespace shared::filesystem {

std::filesystem::path getAppDataPath() {
    return fs::path(shared::getEnv("APPDATA"));
}

std::filesystem::path getLocalAppDataPath() {
    return fs::path(shared::getEnv("LOCALAPPDATA"));
}

std::filesystem::path getSquadOvFolder() {
    return getAppDataPath() / std::filesystem::path("SquadOV" + shared::getEnv("SQUADOV_APPDATA_SUFFIX"));
}

std::filesystem::path getSquadOvTzDataFolder() {
    return getSquadOvFolder() / fs::path("tzdata");
}

std::filesystem::path getSquadOvUserFolder() {
    return fs::path(shared::getEnv("SQUADOV_USER_APP_FOLDER"));
}

std::filesystem::path getSquadOvRecordFolder() {
    const auto appData = getSquadOvUserFolder();
    return appData / std::filesystem::path("Record");
}

std::filesystem::path getSquadOvLogFolder() {
    const auto appData = getSquadOvFolder();
    return appData / std::filesystem::path("Logs");
}

std::filesystem::path getSquadOvServiceLogFolder() {
    const auto appData = getSquadOvLogFolder();
    return appData / std::filesystem::path("Service");
}

std::filesystem::path getAimlabAppDataFolder() {
    return getAppDataPath().parent_path() / fs::path("LocalLow") / fs::path("Statespace") / fs::path("aimlab_tb");
}

std::filesystem::path getHearthstoneAppDataFolder() {
    return getLocalAppDataPath() / fs::path("Blizzard") / fs::path("Hearthstone");
}

}