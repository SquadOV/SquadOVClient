#pragma once

#include "game_event_watcher/csgo/csgo_gsi_packet.h"
#include <CivetServer.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <shared_mutex>

namespace game_event_watcher {

using CsgoGsiPacketDelegate = std::function<void(const CsgoGsiPacket&)>;

class CsgoGsiListener {
public:
    static CsgoGsiListener* singleton();
    static void enableCsgoGsi();

    CsgoGsiListener();
    ~CsgoGsiListener();

    int port() const;
    void handleGsiPacket(const CsgoGsiPacket& packet);

    int64_t addGsiPacketListener(const CsgoGsiPacketDelegate& delegate);
    void removeGsiPacketListener(int64_t id);

    friend class CsgoGsiHandler;
private:
    int getPreExistingPort() const;

    std::unique_ptr<CivetServer> _server;
    std::unique_ptr<CivetHandler> _handler;

    std::shared_mutex _mutex;
    std::unordered_map<int64_t, CsgoGsiPacketDelegate> _delegates;
    int64_t _nextDelegateId = 0;
};
using CsgoGsiListenerPtr = std::unique_ptr<CsgoGsiListener>;

}