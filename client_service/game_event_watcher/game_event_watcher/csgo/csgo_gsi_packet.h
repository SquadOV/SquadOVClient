#pragma once

#include "shared/json.h"
#include "shared/log/log.h"

namespace game_event_watcher {

struct CsgoGsiPacket {

};

std::ostream& operator<<(std::ostream& os, const CsgoGsiPacket& p);

}

namespace shared::json {

template<>
struct JsonConverter<game_event_watcher::CsgoGsiPacket> {
    static game_event_watcher::CsgoGsiPacket from(const nlohmann::json& v) {
        game_event_watcher::CsgoGsiPacket packet;
        LOG_INFO("read csgo gsi packet: " << v.dump(4) << std::endl);
        return packet;
    }
};

}