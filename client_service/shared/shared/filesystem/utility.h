#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include "shared/time.h"

namespace shared::filesystem {

using PathFilter = std::function<bool(const std::filesystem::path&)>;

std::filesystem::path getNewestFileInFolder(const std::filesystem::path& folder, const PathFilter& filter = PathFilter());
std::chrono::seconds secondsSinceLastFileWrite(const std::filesystem::path& pth);
shared::TimePoint timeOfLastFileWrite(const std::filesystem::path& pth);
std::string pathUtf8(const std::filesystem::path& path);
std::filesystem::path generateTimestampBackupFileName(const std::filesystem::path& fname);
void pruneFilesystemPaths(std::vector<std::filesystem::path>& paths, int maxKeep);

}