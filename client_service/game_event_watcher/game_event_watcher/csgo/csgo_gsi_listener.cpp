#include "game_event_watcher/csgo/csgo_gsi_listener.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"

#include <boost/algorithm/string.hpp>

namespace fs = std::filesystem;
namespace game_event_watcher {

class CsgoGsiHandler: public CivetHandler {
public:
    bool handlePost(CivetServer *server, struct mg_connection *conn) {
        return true;
    }
};

CsgoGsiListener* CsgoGsiListener::singleton() {
    static auto global = std::make_unique<CsgoGsiListener>();
    return global.get();
}

CsgoGsiListener::CsgoGsiListener() {
    LOG_INFO("Initializing CS:GO GSI Listener..." << std::endl);
    std::vector<std::string> options = {
        "listening_ports", "127.0.0.1:0",
        "num_threads", "4"
    };
    _server = std::make_unique<CivetServer>(options);
}

CsgoGsiListener::~CsgoGsiListener() {
    LOG_INFO("Shutting Down CS:GO GSI Listener..." << std::endl);
}

int CsgoGsiListener::port() const {
    return _server->getListeningPorts().back();
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