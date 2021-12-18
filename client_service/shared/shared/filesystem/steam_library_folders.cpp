#include "shared/filesystem/steam_library_folders.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/valve/vdf_file.h"

namespace fs = std::filesystem;
namespace shared::filesystem {

SteamLibraryFolders* getSteamLibrarySingleton() {
    static SteamLibraryFoldersPtr singleton = std::make_unique<SteamLibraryFolders>();
    return singleton.get();
}

SteamLibraryFolders::SteamLibraryFolders() {
    initialize();
}

void SteamLibraryFolders::initialize() {
    if (_init) {
        return;
    }

    const auto steamPath = getSteamInstallFolder();
    if (!steamPath) {
        LOG_WARNING("...Can't initialize Steam library folders...no steam path found" << std::endl);
        return;
    }

    const auto libraryPath = steamPath.value() / fs::path("steamapps") / fs::path("libraryfolders.vdf");
    LOG_INFO("Loading Steam Library Folders: " << libraryPath << std::endl);
    if (!fs::exists(libraryPath)) {
        LOG_WARNING("...Can't initialize Steam library folders...no libraryfolders.vdf file found: " << libraryPath << std::endl);
        return;
    }

    const std::vector<SteamGame> allSteamGames = {
        SteamGame::CSGO,
    };

    const auto vdf = shared::valve::readVdfFile(libraryPath);
    const auto& libFolders = vdf["libraryfolders"];
    for (int i = 0;; ++i) {
        const std::string key = std::to_string(i);
        if (!libFolders.hasKey(key)) {
            break;
        }

        const auto& lib = libFolders[key];
        if (!lib.hasKey("path")) {
            continue;
        }

        const auto libPath = lib["path"].value();
        LOG_INFO("Found Steam Library Path: " << libPath << std::endl);

        const auto& apps = lib["apps"];
        for (auto game: allSteamGames) {
            const std::string gameKey = std::to_string(static_cast<int>(game));
            if (!apps.hasKey(gameKey)) {
                continue;
            }
            LOG_INFO("...Has Game: " << gameKey << std::endl);
            _libraryPaths[game] = libPath;
        }
    }
    
    _init = true;
}

std::optional<std::filesystem::path> SteamLibraryFolders::getSteamLibraryPathForGame(SteamGame game) const {
    const auto it = _libraryPaths.find(game);
    if (it == _libraryPaths.end()) {
        return std::nullopt;
    }
    return it->second;
}

}