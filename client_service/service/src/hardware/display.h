#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace service::hardware {

struct Monitor {
    std::string manufacturer;
    std::string name;
    int width = 0;
    int height = 0;
    int year = 0;
    int refresh = 0;

    nlohmann::json toJson() const;
};

struct GPUInfo {
    std::string name;
    size_t memoryBytes = 0;

    nlohmann::json toJson() const;
};

struct Display {
    std::vector<GPUInfo> gpus;
    std::vector<Monitor> monitors;

    nlohmann::json toJson() const;
};

Display getDisplay();

std::ostream& operator<<(std::ostream& os, const Display& p);
std::ostream& operator<<(std::ostream& os, const Monitor& p);
std::ostream& operator<<(std::ostream& os, const GPUInfo& p);

}