#pragma once 
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

namespace service::recorder::audio {

struct AudioDeviceResponse {
    std::vector<std::string> options;
    std::string default;

    nlohmann::json toJson() const;
};

}