#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include "shared/time.h"

namespace shared::aimlab {
    
struct TaskData {
    int taskId = 0;
    std::string klutchId;
    std::string taskName;
    shared::TimePoint createDate;
    int mode = 0;
    int score = 0;
    std::string version;
    nlohmann::json rawData;

    nlohmann::json toJson() const;
};

}