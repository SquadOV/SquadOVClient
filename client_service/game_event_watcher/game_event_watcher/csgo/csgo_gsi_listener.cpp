#include "game_event_watcher/csgo/csgo_gsi_listener.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/http/civetweb_utility.h"
#include "process_watcher/watcher.h"

#include <boost/algorithm/string.hpp>
#include <regex>

namespace fs = std::filesystem;
namespace game_event_watcher {

class CsgoGsiHandler: public CivetHandler {
public:
    explicit CsgoGsiHandler(CsgoGsiListener* listener):
        _listener(listener) {

    }

    bool handlePost(CivetServer* server, struct mg_connection* conn) {
        const auto packet = shared::http::readTypedCivetJsonBody<CsgoGsiPacket>(conn);
        _listener->handleGsiPacket(packet);

        shared::http::writeCivetStatusResponse(shared::http::HttpStatusCode::NoContent, conn);
        return true;
    }

private:
    CsgoGsiListener* _listener;
};

CsgoGsiListener* CsgoGsiListener::singleton() {
    static auto global = std::make_unique<CsgoGsiListener>();
    return global.get();
}

CsgoGsiListener::CsgoGsiListener() {
    LOG_INFO("Initializing CS:GO GSI Listener..." << std::endl);

    // To allow SquadOV to be launched AFTER CS:GO, we need to check to see if a
    // config file exists. If a config file exists **AND** CS:GO is running, we'll
    // use the existing port instead of creating a new one.
    std::ostringstream listeningPort;
    listeningPort << "127.0.0.1:" << getPreExistingPort();

    std::vector<std::string> options = {
        "listening_ports", listeningPort.str(),
        "num_threads", "4"
    };
    _server = std::make_unique<CivetServer>(options);
    LOG_INFO("...Initializing CS:GO GSI Listener on port: " << port() << std::endl);

    _handler = std::make_unique<CsgoGsiHandler>(this);    
    _server->addHandler("/csgo/gsi", *_handler);
}

CsgoGsiListener::~CsgoGsiListener() {
    LOG_INFO("Shutting Down CS:GO GSI Listener..." << std::endl);
}

const std::regex portRegex("\"uri\"\\s\"http:\\/\\/127\\.0\\.0\\.1:(\\d+)\\/csgo\\/gsi\"");
int CsgoGsiListener::getPreExistingPort() const {
    const auto cfgDirectory = shared::filesystem::getCsgoCfgFolder();
    if (!cfgDirectory) {
        return 0;
    }

    const auto destCfgFile = fs::path(cfgDirectory.value()) / fs::path("gamestate_integration_squadov.cfg");
    if (!fs::exists(destCfgFile)) {
        return 0;
    }

    std::ifstream src(destCfgFile);
    std::string line;

    int port = 0;
    while (src.is_open() && std::getline(src, line)) {
        if (line.find("\"uri\"") == std::string::npos) {
            continue;
        }

        std::smatch matches;
        if (!std::regex_search(line, matches, portRegex) || matches.size() < 2) {
            continue;
        }

        port = std::stoi(matches[1].str());
        break;
    }

    return process_watcher::isGameRunning(shared::EGame::CSGO) ? port : 0;
}

int CsgoGsiListener::port() const {
    return _server->getListeningPorts().back();
}

void CsgoGsiListener::enableCsgoGsi() {
    const auto newPort = CsgoGsiListener::singleton()->port();
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
        boost::replace_all(line, "${PORT}", std::to_string(newPort));
        dst << line << std::endl;
    }

    src.close();
    dst.close();
}

int64_t CsgoGsiListener::addGsiPacketListener(const CsgoGsiPacketDelegate& delegate) {
    std::lock_guard guard(_mutex);
    _delegates[_nextDelegateId++] = delegate;
    return _nextDelegateId - 1;
}

void CsgoGsiListener::removeGsiPacketListener(int64_t id) {
    std::lock_guard guard(_mutex);
    const auto it = _delegates.find(id);
    if (it == _delegates.end()) {
        return;
    }
    _delegates.erase(it);
}

void CsgoGsiListener::handleGsiPacket(const CsgoGsiPacket& packet) {
    std::shared_lock guard(_mutex);
    for (const auto& kvp : _delegates) {
        kvp.second(packet);
    }
}

}