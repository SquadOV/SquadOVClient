#pragma once

#include <atomic>
#include <functional>
#include <google/protobuf/message.h>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <steam/steam_api.h>
#include <steam/isteamgamecoordinator.h>

#include "steamworks_interface/gc_delegate.h"

namespace steamworks_interface {

class SteamworksClient {
    using DelegateContainer = std::unordered_multimap<uint32_t, SteamworksGcDelegatePtr>;
public:
    static SteamworksClient* singleton();

    SteamworksClient();
    ~SteamworksClient();

    // Game Coordinator interface.
    void sendGcMessageWithResponse(uint32_t sendType, google::protobuf::Message* message, uint32_t responseType, const SteamworksGcDelegatePtr& delegate);
    void sendGcMessage(uint32_t type, google::protobuf::Message* message);

    DelegateContainer::iterator addGcMessageHandler(uint32_t type, const SteamworksGcDelegatePtr& delegate);
    void removeGcMessageHandler(const DelegateContainer::iterator& it);

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

    std::recursive_mutex _delegateMutex;
    DelegateContainer _delegates;
};

}