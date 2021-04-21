#pragma once

#include <filesystem>
#include <optional>

namespace shared::filesystem {

std::filesystem::path getAppDataPath();
std::filesystem::path getLocalAppDataPath();

std::filesystem::path getSquadOvFolder();
std::filesystem::path getSquadOvUserFolder();
std::filesystem::path getSquadOvTzDataFolder();
std::filesystem::path getSquadOvRecordFolder();
std::filesystem::path getSquadOvClipFolder();
std::filesystem::path getSquadOvLogFolder();
std::filesystem::path getSquadOvServiceLogFolder();
std::filesystem::path getSquadOvUserSettingsFile();
std::filesystem::path getSquadOvServiceDumpFolder();
std::filesystem::path getSquadOvDvrSessionFolder();

std::filesystem::path getAimlabAppDataFolder();

std::filesystem::path getHearthstoneAppDataFolder();

std::filesystem::path getCurrentExeFolder();
std::filesystem::path getSquadOvTempFolder();
std::filesystem::path getCsgoResourcesPath();

std::optional<std::filesystem::path> getSteamInstallFolder();

std::optional<std::filesystem::path> getCsgoInstallFolder();
std::optional<std::filesystem::path> getCsgoCfgFolder();
std::optional<std::filesystem::path> getCsgoAutoExecFile();
std::optional<std::filesystem::path> getCsgoLogFile();

}