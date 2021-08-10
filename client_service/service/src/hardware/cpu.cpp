#include "hardware/cpu.h"

#include "shared/log/log.h"
extern "C" {
#include "libcpuid.h"
}

namespace service::hardware {

CPU getCPU() {
    CPU cpu;
    cpu.brand = "Unknown";
    cpu.vendor = "Unknown";
    cpu.clock = -1;
    cpu.cores = -1;

    if (!cpuid_present()) {
        LOG_WARNING("...CPUID not present." << std::endl);
        return cpu;
    }

    cpu_raw_data_t raw;
    if (cpuid_get_raw_data(&raw) < 0) {
        LOG_WARNING("...Failed to run CPUID." << std::endl);
        return cpu;
    }

    cpu_id_t data;
    if (cpu_identify(&raw, &data) < 0) {
        LOG_WARNING("...Failed to identify CPU." << std::endl);
        return cpu;
    }

    cpu.brand = std::string(data.brand_str);
    cpu.vendor = std::string(data.vendor_str);
    cpu.clock = cpu_clock_by_os();
    cpu.cores = data.num_cores;

    return cpu;
}

nlohmann::json CPU::toJson() const {
    const nlohmann::json obj = {
        { "brand", brand },
        { "vendor", vendor },
        { "clock", clock },
        { "cores", cores }
    };
    return obj;
}

std::ostream& operator<<(std::ostream& os, const CPU& p) {
    os << "Vendor: " << p.vendor << std::endl
       << "Brand: " << p.brand << std::endl
       << "Clock: " << p.clock << "MHz" << std::endl
       << "Cores: "<< p.cores;
    return os;
}

}