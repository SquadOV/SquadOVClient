#pragma once

#include <nlohmann/json.hpp>

namespace shared::squadov {

struct SpeedCheckData {
    float speedMbs;
    nlohmann::json toJson() const;
};

}