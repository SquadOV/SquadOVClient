#pragma once

#include "shared/squadov/speed_check.h"

namespace shared::squadov {

nlohmann::json SpeedCheckData::toJson() const {
    return {
        {"speed_mbps", speed_mbps}
    };
}

SpeedCheckData SpeedCheckData::fromJson(const nlohmann::json& obj) {
    SpeedCheckData speedCheckData;
    speedCheckData.speed_mbps = obj["speed_mbps"].get<double>();
    return speedCheckData;
}

}