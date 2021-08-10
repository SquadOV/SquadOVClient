#pragma once

#include "hardware/os.h"
#include "hardware/cpu.h"
#include "hardware/display.h"

#include <nlohmann/json.hpp>
#include <ostream>
#include <string>

namespace service::hardware {

struct Hardware {
    OperatingSystem os;
    CPU cpu;
    size_t ramKb;
    Display display;

    nlohmann::json toJson() const;
};

Hardware getSystemHardware();

std::ostream& operator<<(std::ostream& os, const Hardware& p);

}