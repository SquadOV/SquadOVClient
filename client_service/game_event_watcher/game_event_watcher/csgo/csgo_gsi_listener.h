#pragma once

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

    friend class CsgoGsiHandler;
private:
    std::unique_ptr<CivetServer> _server;
    CsgoGsiHandler _handler;
};
using CsgoGsiListenerPtr = std::unique_ptr<CsgoGsiListener>;

}