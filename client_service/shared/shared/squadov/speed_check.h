#pragma once

#include <nlohmann/json.hpp>

namespace shared::squadov {

struct SpeedCheckData {
    double speed_mbps;
    
    nlohmann::json toJson() const;
    static SpeedCheckData fromJson(const nlohmann::json& obj);
};

}