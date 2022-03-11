#include <Windows.h>

#include "shared/system/install.h"
#include "shared/log/log.h"
#include "shared/wow/toc.h"
#include "shared/wow/wtf.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/http/http_client.h"
#include <functional>
#include <boost/algorithm/string.hpp>
#include <regex>
#include <libzippp.h>

namespace fs = std::filesystem;
using namespace libzippp;

namespace shared::system {
namespace {
const std::regex COMBAT_LOGGING_REGEX("LoggingCombat\\(\\w+?\\)");

struct RegistryUninstallEntry {
    std::optional<std::wstring> displayIcon;
    std::optional<std::wstring> displayName;
    std::optional<fs::path> installLocation;
    std::optional<fs::path> installSource;
};
using RegistryUninstallCallback = std::function<bool(const RegistryUninstallEntry&)>;

std::optional<RegistryUninstallEntry> getUninstallEntryHelper(const wchar_t* path, const wchar_t* subpath) {
    std::wostringstream subkeyPath;
    subkeyPath << path << "\\" << subpath;

    HKEY subkey;
    auto res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkeyPath.str().c_str(), 0, KEY_READ, &subkey);
    if (res != ERROR_SUCCESS) {
        LOG_WARNING("Failed to open registry key path:" << subkeyPath.str() << std::endl);
        return std::nullopt;
    }

    RegistryUninstallEntry entry;

    {
        WCHAR data[1024];
        DWORD count = 1024;
        DWORD dwType = REG_SZ;
        res = RegGetValueW(subkey, L"", L"DisplayIcon", RRF_RT_REG_SZ, &dwType, data, &count);
        if (res == ERROR_SUCCESS) {
            entry.displayIcon = std::wstring(data);
        }
    }

    {
        WCHAR data[1024];
        DWORD count = 1024;
        DWORD dwType = REG_SZ;
        res = RegGetValueW(subkey, L"", L"DisplayName", RRF_RT_REG_SZ, &dwType, data, &count);
        if (res == ERROR_SUCCESS) {
            entry.displayName = std::wstring(data);
        }
    }

    {
        WCHAR data[1024];
        DWORD count = 1024;
        DWORD dwType = REG_SZ;
        res = RegGetValueW(subkey, L"", L"InstallLocation", RRF_RT_REG_SZ, &dwType, data, &count);
        if (res == ERROR_SUCCESS) {
            entry.installLocation = fs::path(std::wstring(data));
        }
    }

    {
        WCHAR data[1024];
        DWORD count = 1024;
        DWORD dwType = REG_SZ;
        res = RegGetValueW(subkey, L"", L"InstallSource", RRF_RT_REG_SZ, &dwType, data, &count);
        if (res == ERROR_SUCCESS) {
            entry.installSource = fs::path(std::wstring(data));
        }
    }

    RegCloseKey(subkey);
    return entry;
}

std::optional<RegistryUninstallEntry> getUninstallEntry(const wchar_t* subkey) {
    auto entry = getUninstallEntryHelper(L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", subkey);
    if (!entry) {
        return getUninstallEntryHelper(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", subkey);
    }
    return entry;
}
}

std::optional<fs::path> getExecutableInstalledPath(shared::EGame game) {
    LOG_INFO("Getting EXE install path for game: " << shared::gameToString(game) << std::endl);

    // Search for the uninstall entry in the registry to get the install location
    fs::path installLocation;

    std::optional<RegistryUninstallEntry> entry;
    switch (game) {
        case shared::EGame::WoW:
            entry = getUninstallEntry(L"World of Warcraft");
            break;
        case shared::EGame::WowTbc:
            entry = getUninstallEntry(L"Burning Crusade Classic");
            break;
        case shared::EGame::WowVanilla:
            entry = getUninstallEntry(L"World of Warcraft Classic Era");
            break;
    }

    if (!entry) {
        LOG_WARNING("No uninstall entry for game: " << shared::gameToString(game) << std::endl);
        return std::nullopt;
    }
    
    if (entry.value().installLocation) {
        installLocation = entry.value().installLocation.value();
    }
  
    if (!fs::exists(installLocation)) {
        LOG_WARNING("Invalid install location: " << installLocation << std::endl);
        return std::nullopt;
    }

    fs::path exeLocation;
    switch (game) {
        case shared::EGame::WoW:
            exeLocation = installLocation / fs::path("_retail_") / fs::path("Wow.exe");
            break;
        case shared::EGame::WowTbc:
            exeLocation = installLocation / fs::path("_classic_") / fs::path("WowClassic.exe");
            break;
        case shared::EGame::WowVanilla:
            exeLocation = installLocation / fs::path("_classic_era_") / fs::path("WowClassic.exe");
            break;
        default:
            return std::nullopt;    
    }

    if (!fs::exists(exeLocation)) {
        LOG_WARNING("Invalid EXE location: " << exeLocation << std::endl);
        return std::nullopt;
    }

    return exeLocation;
}

bool hasWowCombatLogAddonInstalled(const std::filesystem::path& exePath, shared::EWowRelease release) {
    const fs::path addonDirectory = exePath.parent_path() / fs::path("Interface") / fs::path("Addons");
    if (!fs::exists(addonDirectory)) {
        return false;   
    }

    bool ret = false;
    for (const auto& entry : fs::directory_iterator(addonDirectory)) {
        if (!entry.is_directory()) {
            continue;
        }

        LOG_INFO("Checking AddOn: " << entry << std::endl);

        // Find the .toc file
        const fs::path tocFilePath = entry / fs::path(entry.path().filename().native() + L".toc");
        if (!fs::exists(tocFilePath)) {
            LOG_WARNING("...No TOC File Exists: " << tocFilePath << std::endl);
            continue;
        }

        const auto tocFile = shared::wow::TocFile::loadFromFile(tocFilePath);
        // Make sure the interface is correct (if expecting retail, then the toc must indicate it's a retail addon etc).
        if (tocFile.wowInterface() != release) {
            LOG_WARNING("...Addon TOC interface mismatch." << std::endl);
            continue;
        }

        // Now we need to dig through the main code files and see if they ever call "LoggingCombat(BOOL)".
        for (const auto& codeFilePath: tocFile.codeFiles()) {
            const fs::path fname = entry / fs::path(codeFilePath);
            if (!fs::exists(fname)) {
                LOG_WARNING("...Code file not found: " << fname << std::endl);
                continue;
            }

            std::ifstream f(fname);
            std::string line;
            bool goNext = false;
            while (std::getline(f, line)) {
                if (std::regex_search(line, COMBAT_LOGGING_REGEX)) {
                    LOG_INFO("\tFOUND COMBAT LOG ADDON." << std::endl);
                    if (ret) {
                        LOG_WARNING("!!!!!!! FOUND A DUPLICATE COMBAT LOGGING ADDON !!!!!!!" << std::endl);
                    }
                    goNext = true;
                    ret = true;
                    break;
                }
            }

            if (goNext) {
                break;
            }
        }
    }
    return ret;
}

void installCombatLogAddonForWow(const std::filesystem::path& exePath, shared::EWowRelease release) {
    const fs::path addonDirectory = exePath.parent_path() / fs::path("Interface") / fs::path("Addons");
    if (!fs::exists(addonDirectory)) {
        fs::create_directories(addonDirectory); 
    }

    std::string addonName;
    std::string addonDownloadUrl;
    std::string subFolder;

    switch (release) {
        case shared::EWowRelease::Retail:
            addonName = "SimpleCombatLogger";
            addonDownloadUrl = "https://github.com/csutcliff/SimpleCombatLogger/archive/refs/tags/1.5.2.zip";
            subFolder = "SimpleCombatLogger-1.5.2";
            break;
        case shared::EWowRelease::Tbc:
            addonName = "AutoCombatLogger";
            addonDownloadUrl = "https://github.com/Talryn/AutoCombatLogger/releases/download/9.2.1/AutoCombatLogger-9.2.1.zip";
            subFolder = "AutoCombatLogger";
            break;
        case shared::EWowRelease::Vanilla:
            addonName = "AutoCombatLogger";
            addonDownloadUrl = "https://github.com/Talryn/AutoCombatLogger/releases/download/9.2.1/AutoCombatLogger-9.2.1.zip";
            subFolder = "AutoCombatLogger";
            break;
    }

    const fs::path dlLocation = shared::filesystem::getSquadOvTempFolder() / fs::path(addonName + ".zip");
    LOG_INFO("DL addon to: " << dlLocation << std::endl);
    if (!shared::http::downloadFileToLocation(addonDownloadUrl, dlLocation, shared::http::DownloadUploadProgressFn{})) {
        LOG_ERROR("Failed to download addon from URI: " << addonDownloadUrl << std::endl);
        return;
    }

    const fs::path extractFolder = shared::filesystem::getSquadOvTempFolder() / fs::path(addonName);
    if (fs::exists(extractFolder)) {
        fs::remove_all(extractFolder);
        fs::create_directories(extractFolder);
    }

    ZipArchive archive(shared::filesystem::pathUtf8(dlLocation));
    if (!archive.open(ZipArchive::ReadOnly)) {
        LOG_ERROR("Failed to open ZIP archive: " << dlLocation << std::endl);
        return;
    }

    auto entries = archive.getEntries();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        const auto name = it->getName();

        if (it->isDirectory()) {
            fs::create_directories(extractFolder / fs::path(name));
        } else if (it->isFile()) {
            std::ofstream of(extractFolder / fs::path(name), std::ofstream::binary);
            if (it->readContent(of) != LIBZIPPP_OK) {
                LOG_ERROR("Failed to read content of ZIP file:" << name << std::endl);
                return;
            }
        }
    }

    archive.close();
    
    const auto workDir = extractFolder / fs::path(subFolder);
    const auto targetDir = addonDirectory / fs::path(addonName);
    if (fs::exists(targetDir)) {
        fs::remove_all(targetDir);
    }

    switch (release) {
        case shared::EWowRelease::Retail:
            fs::copy(workDir, targetDir);
            break;
        case shared::EWowRelease::Tbc:
            fs::rename(workDir / fs::path("AutoCombatLogger_TBC.toc"), workDir / fs::path("AutoCombatLogger.toc"));
            fs::copy(workDir, targetDir);
            break;
        case shared::EWowRelease::Vanilla:
            fs::rename(workDir / fs::path("AutoCombatLogger_Vanilla.toc"), workDir / fs::path("AutoCombatLogger.toc"));
            fs::copy(workDir, targetDir);
            break;
    }

    fs::remove_all(dlLocation);
    fs::remove_all(extractFolder);
}

bool hasWowAdvancedCombatLoggingTurnedOn(const std::filesystem::path& exePath) {
    const auto wtfPath = exePath.parent_path() / fs::path("WTF") / fs::path("Config.wtf");
    const auto wtfFile = shared::wow::WtfFile::loadFromFile(wtfPath);
    return wtfFile.hasSetting("advancedCombatLogging") && (wtfFile.getSetting("advancedCombatLogging") == "1");
}

void turnOnAdvancedCombatLoggingForWow(const std::filesystem::path& exePath) {
    const auto wtfPath = exePath.parent_path() / fs::path("WTF") / fs::path("Config.wtf");
    auto wtfFile = shared::wow::WtfFile::loadFromFile(wtfPath);
    wtfFile.setSetting("advancedCombatLogging", "1");
    wtfFile.saveToFile(wtfPath);
}

}