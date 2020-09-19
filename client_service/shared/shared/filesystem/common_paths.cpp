#include "shared/filesystem/common_paths.h"

namespace shared::filesystem {

std::filesystem::path getSquadOvFolder() {
#ifdef _WIN32
    char* appData;
    size_t len;
    _dupenv_s(&appData, &len, "APPDATA");
    const std::filesystem::path appDataDir(appData);
    free(appData);
#else
    throw std::runtime_error("Unsupported OS.");
#endif
    return appDataDir / std::filesystem::path("SquadOV");;
}

std::filesystem::path getSquadOvVideoFolder() {
    const auto appData = getSquadOvFolder();
    return appData / std::filesystem::path("Videos");
}

}