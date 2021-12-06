#pragma once 
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

namespace service::recorder::audio {

struct SingleAudioDevice {
    SingleAudioDevice() = default;
    SingleAudioDevice(const std::string& inName, const std::string& inId):
        name(inName),
        id(inId)
    {}

    std::string name;
    std::string id;

    nlohmann::json toJson() const;
};

struct AudioDeviceResponse {
    std::vector<SingleAudioDevice> options;
    SingleAudioDevice default;

    nlohmann::json toJson() const;
};

}