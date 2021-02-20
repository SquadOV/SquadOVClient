#pragma once

#include <stdint.h>
#include <nlohmann/json.hpp>
#include <optional>

namespace shared::hearthstone {

struct HearthstoneRatings {
    std::optional<int32_t> battlegroundsRating;
    std::optional<int32_t> duelsCasualRating;
    std::optional<int32_t> duelsHeroicRating;

    nlohmann::json toJson() const;
};

}