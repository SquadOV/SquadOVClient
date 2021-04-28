#pragma once

#include <chrono>
#include <filesystem>
#include <optional>

namespace shared::process {

int runProcessWithTimeout(const std::filesystem::path& exe, std::string commandLine, const std::optional<std::chrono::seconds>& timeoutSeconds);

}