#include "game_event_watcher/csgo/csgo_gsi_packet.h"

namespace game_event_watcher {

std::ostream& operator<<(std::ostream& os, const CsgoGsiPacket& p) {
    os << "CsgoGsiPacket {" << std::endl
        << "}";
    return os;
}

}