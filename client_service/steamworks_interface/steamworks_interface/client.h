#pragma once

#include <atomic>
#include <google/protobuf/message.h>
#include <thread>
#include <steam/steam_api.h>
#include <steam/isteamgamecoordinator.h>

namespace steamworks_interface {

class SteamworksClient {
public:
    static SteamworksClient* singleton();

    SteamworksClient();
    ~SteamworksClient();

    // Game Coordinator interface.
    void sendGcMessage(uint32_t type, google::protobuf::Message* message);
    void addGcMessageHandler();

private:
    void cleanup();
    void waitForConnect();

    std::thread _callbackThread;
    std::atomic<bool> _running;

    ISteamGameCoordinator* _gcCoordinator = nullptr;

    // Callback for whenever the game coordinator (GC) has a message for us.
    void onGcMessageAvailable(GCMessageAvailable_t* msg);
    CCallback<SteamworksClient, GCMessageAvailable_t, false> _gcMessageAvailableCb;
    // Callback for whenever we failed to send a message to the GC.
    void onGcMessageFail(GCMessageFailed_t* msg);
    CCallback<SteamworksClient, GCMessageFailed_t, false> _gcMessageFailedCb;
};

}