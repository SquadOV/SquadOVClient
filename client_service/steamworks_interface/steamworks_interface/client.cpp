#include "steamworks_interface/client.h"
#include "shared/errors/error.h"
#include <memory>
#include <stdexcept>
#include <vector>

#ifdef SendMessage
#undef SendMessage
#endif

extern "C" void __cdecl steamApiLoggingHook(int severity, const char* text) {
    if (severity == 0) {
        LOG_INFO("[STEAM] - " << text << std::endl);
    } else {
        LOG_WARNING("[STEAM] - " << text << std::endl);
    }
}

namespace steamworks_interface {
namespace {

constexpr uint32_t GC_PROTO_MASK = 0x80000000;

std::string steamGcResultToString(EGCResults result) {
    switch (result) {
        case k_EGCResultOK:
            return "OK";
        case k_EGCResultNoMessage:
            return "No Message in queue";
        case k_EGCResultBufferTooSmall:
            return "Buffer too small";
        case k_EGCResultNotLoggedOn:
            return "Not logged in";
        case k_EGCResultInvalidMessage:
            return "Invalid message";
    }
    return "";
}

}

SteamworksClient* SteamworksClient::singleton() {
    static auto client = std::make_unique<SteamworksClient>();
    return client.get();
}

SteamworksClient::SteamworksClient():
    _gcMessageAvailableCb(this, &SteamworksClient::onGcMessageAvailable),
    _gcMessageFailedCb(this, &SteamworksClient::onGcMessageFail) {
    try {
        if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid)) {
            THROW_ERROR("Failed to restart app in Steam.");
        }

        if (!SteamAPI_Init()) {
            THROW_ERROR("Failed to initialize Steam API.");
        }

        if (!SteamUser()->BLoggedOn()) {
            THROW_ERROR("User is not logged into Steam.");
        }

        // Retrieve interfaces that we need from Steam
        _gcCoordinator = reinterpret_cast<ISteamGameCoordinator*>(SteamClient()->GetISteamGenericInterface(
            SteamAPI_GetHSteamUser(),
            SteamAPI_GetHSteamPipe(),
            STEAMGAMECOORDINATOR_INTERFACE_VERSION
        ));

        if (!_gcCoordinator) {
            THROW_ERROR("Failed to obtain Steam GC coordinator.");
        }

        _running = true;
        _callbackThread = std::thread([this](){
            while (_running) {
                SteamAPI_RunCallbacks();

                // Emulate a game in a sense by having this run at 60fps.
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        });

        // Hook logs
        SteamUtils()->SetWarningMessageHook(&steamApiLoggingHook);
    } catch (std::exception& ex) {
        cleanup();
        throw ex;
    }
}

SteamworksClient::~SteamworksClient() {
    cleanup();
}

void SteamworksClient::cleanup() {
    _running = false;
    if (_callbackThread.joinable()) {
        _callbackThread.join();
    }

    SteamAPI_Shutdown();
}

void SteamworksClient::sendGcMessage(uint32_t type, google::protobuf::Message* message) {
    const auto protoMaskedMessageType = type | GC_PROTO_MASK;
    // From what I can tell, the byte buffer we want to end up sending to the game coordinator is composed of two parts
    //  1) The header.
    //  2) The protobuf message.
    //
    // The header is composed of 3 parts:
    //  1) Message flag.
    //  2) Header size (bytes).
    //  3) Header.
    // We can probably ignore the header for now and send a 0 byte header.
    const auto headerBytes = 2 * sizeof(uint32_t);
    const auto messageBytes = message->ByteSizeLong();
    const auto totalBytes = headerBytes + messageBytes;

    std::vector<char> buffer(totalBytes);
    reinterpret_cast<uint32_t*>(buffer.data())[0] = protoMaskedMessageType;
    reinterpret_cast<uint32_t*>(buffer.data())[1] = 0;
    message->SerializeToArray(buffer.data() + headerBytes, messageBytes);

    // Send game coordinator message using the filled out buffer.
    if (!_gcCoordinator) {
        THROW_ERROR("GC coordinator not initialized?");
    }

    const auto res = _gcCoordinator->SendMessage(protoMaskedMessageType, reinterpret_cast<const void*>(buffer.data()), buffer.size());
    if (res != k_EGCResultOK) {
        THROW_ERROR("Failed to send GC message: " << steamGcResultToString(res));
    }
}

void SteamworksClient::addGcMessageHandler() {

}

void SteamworksClient::onGcMessageAvailable(GCMessageAvailable_t* msg) {

}

void SteamworksClient::onGcMessageFail(GCMessageFailed_t* msg) {

}

}