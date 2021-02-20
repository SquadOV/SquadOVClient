#pragma once

#include "shared/time.h"
#include <string>
namespace game_event_watcher {

struct HearthstoneRawLog {
    shared::TimePoint tm;
    std::string section;
    std::string log;
    bool canParse = false;
};


}