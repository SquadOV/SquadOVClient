#include "shared/filesystem/common_paths.h"

#include "shared/errors/error.h"
#include "shared/env.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = std::filesystem;
using namespace std::string_literals;

namespace shared::filesystem {

std::filesystem::path getAppDataPath() {
    return fs::path(shared::getEnvW(L"APPDATA"s));
}

std::filesystem::path getLocalAppDataPath() {
    return fs::path(shared::getEnvW(L"LOCALAPPDATA"s));
}

std::filesystem::path getSquadOvFolder() {
    return getAppDataPath() / std::filesystem::path("SquadOV" + shared::getEnv("SQUADOV_APPDATA_SUFFIX"));
}

std::filesystem::path getSquadOvUserFolder() {
    return fs::path(shared::getEnvW(L"SQUADOV_USER_APP_FOLDER"s));
}

std::filesystem::path getSquadOvTzDataFolder() {
    return fs::path(shared::getEnvW(L"SQUADOV_TZDATA"s));
}

std::filesystem::path getSquadOvUserSettingsFile() {
    return getSquadOvUserFolder() / fs::path("settings.json");
}

std::filesystem::path getSquadOvRecordFolder() {
    const auto appData = getSquadOvUserFolder();
    return appData / std::filesystem::path("Record");
}

std::filesystem::path getSquadOvClipFolder() {
    const auto appData = getSquadOvUserFolder();
    return appData / std::filesystem::path("Clips");
}

std::filesystem::path getSquadOvLogFolder() {
    const auto appData = getSquadOvFolder();
    return appData / std::filesystem::path("Logs");
}

std::filesystem::path getSquadOvServiceLogFolder() {
    const auto appData = getSquadOvLogFolder();
    return appData / std::filesystem::path("Service");
}

std::filesystem::path getSquadOvServiceDumpFolder() {
    const auto appData = getSquadOvLogFolder();
    return appData / std::filesystem::path("Dumps");
}

std::filesystem::path getSquadOvDvrSessionFolder() {
    return getSquadOvRecordFolder();
}

std::filesystem::path getAimlabAppDataFolder() {
    return getAppDataPath().parent_path() / fs::path("LocalLow") / fs::path("Statespace") / fs::path("aimlab_tb");
}

std::filesystem::path getHearthstoneAppDataFolder() {
    return getLocalAppDataPath() / fs::path("Blizzard") / fs::path("Hearthstone");
}

std::filesystem::path getCurrentExeFolder() {
#ifdef _WIN32
    static std::filesystem::path path = [](){
        char path[MAX_PATH];
        DWORD pathLength = GetModuleFileNameA(NULL, path, MAX_PATH);
        if (!pathLength) {
            THROW_ERROR("Failed to get EXE path.");
        }
        std::string str(path, pathLength);
        return std::filesystem::path(str).parent_path();
    }();
    return path;
#endif
}

std::filesystem::path getCsgoResourcesPath() {
    return fs::path(shared::getEnvW(L"SQUADOV_CSGO_RESOURCES"s));
}

std::filesystem::path getSquadOvTempFolder() {
    return fs::temp_directory_path() / fs::path("SquadOV");
}

std::optional<std::filesystem::path> getSteamInstallFolder() {
    // According to the Valve GSI documentation we can find
    // the steam folder under the registry value: HKEY_CURRENT_USER\SOFTWARE\Valve\Steam
    // with the key "SteamPath".
    HKEY key;
    auto res = RegOpenKeyExW(HKEY_CURRENT_USER , L"SOFTWARE\\Valve\\Steam", 0, KEY_QUERY_VALUE, &key);
    if (res != ERROR_SUCCESS) {
        LOG_WARNING("Failed to find Steam Directory: " << res << std::endl);
        return std::nullopt;
    }

    WCHAR path[1024];
    DWORD count = 1024;
    DWORD dwType = REG_SZ;
    res = RegGetValueW(key, L"", L"SteamPath", RRF_RT_REG_SZ, &dwType, path, &count);
    if (res != ERROR_SUCCESS) {
        LOG_WARNING("Failed to get Steam Path: " << res << std::endl);
        return std::nullopt;
    }

    return fs::path(std::wstring(path));
}

std::optional<std::filesystem::path> getCsgoInstallFolder() {
    const auto steamPath = getSteamInstallFolder();
    if (!steamPath.has_value()) {
        return std::nullopt;
    }
    return steamPath.value() / fs::path("steamapps") / fs::path("common") / fs::path("Counter-Strike Global Offensive");
}

std::optional<std::filesystem::path> getCsgoCfgFolder() {
    const auto path = getCsgoInstallFolder();
    if (!path.has_value()) {
        return std::nullopt;
    }
    return path.value() / fs::path("csgo") / fs::path("cfg");
}

std::optional<std::filesystem::path> getCsgoAutoExecFile() {
    const auto path = getCsgoCfgFolder();
    if (!path.has_value()) {
        return std::nullopt;
    }
    return path.value() / fs::path("autoexec.cfg");
}

std::optional<std::filesystem::path> getCsgoLogFile() {
    const auto path = getCsgoInstallFolder();
    if (!path.has_value()) {
        return std::nullopt;
    }
    return path.value() / fs::path("csgo") / fs::path("squadov.log");
}

}