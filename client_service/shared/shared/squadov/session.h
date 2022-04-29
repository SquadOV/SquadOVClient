#pragma once

#include "shared/squadov/user.h"
#include <optional>

namespace shared::squadov {

struct SquadOVSessionStorage {
    std::string sessionId;
    std::optional<SquadOVUser> user;
};

}