#pragma once

#include <string>

namespace shared::riot {

struct RiotUser {
    std::string puuid;
    std::string username;
    std::string tag;
};

}