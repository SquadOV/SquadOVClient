#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>

#include "shared/valve/vdf_file.h"

namespace shared::filesystem {

enum class SteamGame {
    CSGO = 730
};

class SteamLibraryFolders {
public:
    SteamLibraryFolders();

    // Actually read the VDF on disk and parse its contents.
    void initialize();

    std::optional<std::filesystem::path> getSteamLibraryPathForGame(SteamGame game) const;

private:
    bool _init = false;
    std::unordered_map<SteamGame, std::filesystem::path> _libraryPaths;
};

using SteamLibraryFoldersPtr = std::unique_ptr<SteamLibraryFolders>;
SteamLibraryFolders* getSteamLibrarySingleton();

}