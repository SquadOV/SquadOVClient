#pragma once

#include <string>

namespace shared::riot {

struct RiotUser {
    std::string puuid;
    std::string username;
    std::string tag;
};

inline bool operator==(const RiotUser& a, const RiotUser& b) {
    return a.puuid == b.puuid &&
        a.username == b.username &&
        a.tag == b.tag;
}

}