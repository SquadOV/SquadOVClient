#include "shared/filesystem/common_paths.h"

#include "shared/errors/error.h"

namespace fs = std::filesystem;
namespace shared::filesystem {

std::filesystem::path getAppDataPath() {
#ifdef _WIN32
    char* appData;
    size_t len;
    _dupenv_s(&appData, &len, "APPDATA");
    const std::filesystem::path appDataDir(appData);
    free(appData);
#else
    THROW_ERROR("Unsupported OS.");
#endif
    return appDataDir;
}

std::filesystem::path getSquadOvFolder() {
    return getAppDataPath() / std::filesystem::path("SquadOV");;
}

std::filesystem::path getSquadOvRecordFolder() {
    const auto appData = getSquadOvFolder();
    return appData / std::filesystem::path("Record");
}

std::filesystem::path getAimlabAppDataFolder() {
    return getAppDataPath().parent_path() / fs::path("LocalLow") / fs::path("Statespace") / fs::path("aimlab_tb");
}

}