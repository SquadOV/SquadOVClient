#include "game_event_watcher/csgo/csgo_gsi_listener.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"

#include <boost/algorithm/string.hpp>

namespace fs = std::filesystem;
namespace game_event_watcher {

CsgoGsiListener* CsgoGsiListener::singleton() {
    static auto global = std::make_unique<CsgoGsiListener>();
    return nullptr;
}

CsgoGsiListener::CsgoGsiListener() {
    LOG_INFO("Initializing CS:GO GSI Listener..." << std::endl);
}

CsgoGsiListener::~CsgoGsiListener() {
    LOG_INFO("Shutting Down CS:GO GSI Listener..." << std::endl);
}

int CsgoGsiListener::port() const {
    return 1234;
}

void CsgoGsiListener::enableCsgoGsi() {
    const auto cfgDirectory = shared::filesystem::getCsgoCfgFolder();
    if (!cfgDirectory.has_value() || !fs::exists(cfgDirectory.value())) {
        LOG_WARNING("Could not find CSGO cfg directory for GSI.");
        return;
    }

    const auto srcCfgFile = shared::filesystem::getCsgoResourcesPath() / fs::path("gamestate_integration_squadov.cfg");
    const auto destCfgFile = fs::path(cfgDirectory.value()) / fs::path("gamestate_integration_squadov.cfg");

    // We need to replace the port with the one that we'll actually be listening to.
    std::ifstream src(srcCfgFile);
    std::ofstream dst(destCfgFile);

    std::string line;
    while (src.is_open() && std::getline(src, line)) {
        boost::replace_all(line, "${PORT}", std::to_string(CsgoGsiListener::singleton()->port()));
        dst << line << std::endl;
    }

    src.close();
    dst.close();
}

}