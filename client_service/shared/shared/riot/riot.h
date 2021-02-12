#pragma once

#include "shared/time.h"

#include <string>

namespace shared::riot {

struct RiotUser {
    std::string puuid;
    std::string username;
    std::string tag;
};

struct RiotRsoToken {
    std::string puuid;
    std::string rsoToken;
    std::string entitlementsToken;
    shared::TimePoint expires;
};

inline bool operator==(const RiotUser& a, const RiotUser& b) {
    return a.puuid == b.puuid &&
        a.username == b.username &&
        a.tag == b.tag;
}

}