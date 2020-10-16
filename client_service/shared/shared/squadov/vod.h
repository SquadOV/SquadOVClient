#pragma once

#include "shared/time.h"
#include <string>

namespace shared::squadov {

struct VodAssociation {
    std::string matchUuid;
    std::string userUuid;
    std::string videoUuid;
    shared::TimePoint startTime;
    shared::TimePoint endTime;
};

}