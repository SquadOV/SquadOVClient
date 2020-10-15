#pragma once

#include <string>
#include <stdint.h>

namespace shared::squadov {

struct SquadOVUser {
    int64_t id = -1;
    std::string username;
    std::string email;
    bool verified = false;
    std::string uuid;
};

}