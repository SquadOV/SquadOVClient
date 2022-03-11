#pragma once

#include <filesystem>
#include <optional>

#include "shared/games.h"

namespace shared::system {

std::optional<std::filesystem::path> getExecutableInstalledPath(shared::EGame game);

bool hasWowCombatLogAddonInstalled(const std::filesystem::path& exePath, shared::EWowRelease release);
void installCombatLogAddonForWow(const std::filesystem::path& exePath, shared::EWowRelease release);

bool hasWowAdvancedCombatLoggingTurnedOn(const std::filesystem::path& exePath);
void turnOnAdvancedCombatLoggingForWow(const std::filesystem::path& exePath);

}