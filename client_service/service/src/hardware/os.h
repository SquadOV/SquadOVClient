#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace service::hardware {

struct OperatingSystem {
    std::string name;
    std::string majorVersion;
    std::string minorVersion;
    std::string edition;

    nlohmann::json toJson() const;
};

OperatingSystem getOS();

std::ostream& operator<<(std::ostream& os, const OperatingSystem& p);

}