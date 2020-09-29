#pragma once

#include <filesystem>

namespace shared::filesystem {

std::filesystem::path getAppDataPath();

std::filesystem::path getSquadOvFolder();
std::filesystem::path getSquadOvRecordFolder();

std::filesystem::path getAimlabAppDataFolder();

}