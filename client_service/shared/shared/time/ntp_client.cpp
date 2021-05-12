#include "shared/time/ntp_client.h"

#include <memory>

namespace shared::time {

NTPClient* singleton() {
    static auto global = std::make_unique<NTPClient>();
    return global.get();    
}

void NTPClient::initialize() {

}

}