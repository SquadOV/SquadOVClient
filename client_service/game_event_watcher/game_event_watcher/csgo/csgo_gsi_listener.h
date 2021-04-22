#pragma once

#include "game_event_watcher/csgo/csgo_gsi_packet.h"
#include <CivetServer.h>
#include <memory>

namespace game_event_watcher {

class CsgoGsiListener {
public:
    static CsgoGsiListener* singleton();
    static void enableCsgoGsi();

    CsgoGsiListener();
    ~CsgoGsiListener();

    int port() const;
    void handleGsiPacket(const CsgoGsiPacket& packet);

    friend class CsgoGsiHandler;
private:
    std::unique_ptr<CivetServer> _server;
    std::unique_ptr<CivetHandler> _handler;
};
using CsgoGsiListenerPtr = std::unique_ptr<CsgoGsiListener>;

}