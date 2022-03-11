#include "shared/log/log.h"
#include "system/settings.h"
#include "shared/system/install.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    shared::log::Log::initializeGlobalLogger("automated_game_setup.log");

    LOG_INFO("Starting Automated Game Setup..." << std::endl);
    service::system::getCurrentSettings()->reloadSettingsFromFile();

    LOG_INFO("Checking for install: WoW (retail)..." << std::endl);
    const auto wowRetailPath = shared::system::getExecutableInstalledPath(shared::EGame::WoW);
    if (wowRetailPath) {
        LOG_INFO("...WoW Retail Installed: " << wowRetailPath.value() << std::endl);
        if (!shared::system::hasWowCombatLogAddonInstalled(wowRetailPath.value(), shared::EWowRelease::Retail)) {
            LOG_INFO("\t...No Combat Log Addon Installed - Installing." << std::endl);
            shared::system::installCombatLogAddonForWow(wowRetailPath.value(), shared::EWowRelease::Retail);
        }

        if (!shared::system::hasWowAdvancedCombatLoggingTurnedOn(wowRetailPath.value())) {
            LOG_INFO("\t...Turning on advanced combat log." << std::endl);
            shared::system::turnOnAdvancedCombatLoggingForWow(wowRetailPath.value());
        }
    }

    LOG_INFO("Checking for install: WoW (tbc)..." << std::endl);
    const auto wowTbcPath = shared::system::getExecutableInstalledPath(shared::EGame::WowTbc);
    if (wowTbcPath) {
        LOG_INFO("...WoW TBC Installed: " << wowTbcPath.value() << std::endl);
        if (!shared::system::hasWowCombatLogAddonInstalled(wowTbcPath.value(), shared::EWowRelease::Tbc)) {
            LOG_INFO("\t...No Combat Log Addon Installed - Installing." << std::endl);
            shared::system::installCombatLogAddonForWow(wowTbcPath.value(), shared::EWowRelease::Tbc);
        }

        if (!shared::system::hasWowAdvancedCombatLoggingTurnedOn(wowTbcPath.value())) {
            LOG_INFO("\t...Turning on advanced combat log." << std::endl);
            shared::system::turnOnAdvancedCombatLoggingForWow(wowTbcPath.value());
        }
    }

    LOG_INFO("Checking for install: WoW (vanilla)..." << std::endl);
    const auto wowVanillaPath = shared::system::getExecutableInstalledPath(shared::EGame::WowVanilla);
    if (wowVanillaPath) {
        LOG_INFO("...WoW Vanilla Installed: " << wowVanillaPath.value() << std::endl);
        if (!shared::system::hasWowCombatLogAddonInstalled(wowVanillaPath.value(), shared::EWowRelease::Vanilla)) {
            LOG_INFO("\t...No Combat Log Addon Installed - Installing." << std::endl);
            shared::system::installCombatLogAddonForWow(wowVanillaPath.value(), shared::EWowRelease::Vanilla);
        }

        if (!shared::system::hasWowAdvancedCombatLoggingTurnedOn(wowVanillaPath.value())) {
            LOG_INFO("\t...Turning on advanced combat log." << std::endl);
            shared::system::turnOnAdvancedCombatLoggingForWow(wowVanillaPath.value());
        }
    }

    return 0;
}