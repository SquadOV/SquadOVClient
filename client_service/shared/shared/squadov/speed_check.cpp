#pragma once

#include "shared/squadov/speed_check.h"

namespace shared::squadov {

nlohmann::json SpeedCheckData::toJson() const {
    return {
        {"speed_mbps", speed_mbps}
    };
}

}