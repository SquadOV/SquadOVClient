#pragma once

#include "shared/time.h"
#include <string>
#include <nlohmann/json.hpp>

namespace shared::squadov {

struct VodAssociation {
    std::string matchUuid;
    std::string userUuid;
    std::string videoUuid;
    shared::TimePoint startTime;
    shared::TimePoint endTime;

    nlohmann::json toJson() const;
};

struct VodMetadata {
    std::string videoUuid;
    int resX;
    int resY;

    int64_t minBitrate;
    int64_t avgBitrate;
    int64_t maxBitrate;

    int fps;

    std::string id;
    nlohmann::json toJson() const;
};

}