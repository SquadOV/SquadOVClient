#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace service::hardware {

struct CPU {
    std::string vendor;
    std::string brand;
    int clock;
    int cores;

    nlohmann::json toJson() const;
};

CPU getCPU();

std::ostream& operator<<(std::ostream& os, const CPU& p);

}