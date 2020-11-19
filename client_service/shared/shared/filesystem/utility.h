#pragma once

#include <chrono>
#include <filesystem>
#include <functional>

namespace shared::filesystem {

using PathFilter = std::function<bool(const std::filesystem::path&)>;

std::filesystem::path getNewestFileInFolder(const std::filesystem::path& folder, const PathFilter& filter = PathFilter());
std::chrono::seconds secondsSinceLastFileWrite(const std::filesystem::path& pth);

}