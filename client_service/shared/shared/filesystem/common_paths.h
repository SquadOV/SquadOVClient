#pragma once

#include <filesystem>

namespace shared::filesystem {

std::filesystem::path getAppDataPath();

std::filesystem::path getSquadOvFolder();
std::filesystem::path getSquadOvUserFolder();
std::filesystem::path getSquadOvRecordFolder();
std::filesystem::path getSquadOvLogFolder();
std::filesystem::path getSquadOvServiceLogFolder();

std::filesystem::path getAimlabAppDataFolder();

}