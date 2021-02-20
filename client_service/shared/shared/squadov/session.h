#pragma once

#include "shared/squadov/user.h"

namespace shared::squadov {

struct SquadOVSessionStorage {
    std::string sessionId;
    SquadOVUser user;
};

}