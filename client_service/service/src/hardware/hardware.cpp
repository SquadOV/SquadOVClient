#include "hardware/hardware.h"
#include "shared/errors/error.h"


#ifdef _WIN32
#include <Windows.h>
#endif

namespace service::hardware {

Hardware getSystemHardware() {
    Hardware sys;
    sys.os = getOS();
    sys.cpu = getCPU();
    sys.display = getDisplay();

    if (!GetPhysicallyInstalledSystemMemory(&sys.ramKb)) {
        LOG_WARNING("Failed to get amount of RAM: " << shared::errors::getWin32ErrorAsString() << std::endl);
    }

    return sys;
}

nlohmann::json Hardware::toJson() const {
    const nlohmann::json obj = {
        { "os", os.toJson() },
        { "cpu", cpu.toJson() },
        { "ramKb", ramKb },
        { "display", display.toJson() }
    };
    return obj;
}

std::ostream& operator<<(std::ostream& os, const Hardware& p) {
    os << "=================== OPERATING SYSTEM ===================" << std::endl
       << p.os << std::endl
       << "RAM: " << p.ramKb << "Kb" << std::endl
       << "=================== CPU ===================" << std::endl
       << p.cpu << std::endl
       << "=================== Display ===================" << std::endl
       << p.display << std::endl;
    return os;
}

}