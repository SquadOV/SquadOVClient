#include "shared/http/dns_manager.h"

#include <boost/algorithm/string.hpp>

namespace shared::http {

DnsManager* getDnsManager() {
    static DnsManagerPtr manager = std::make_unique<DnsManager>();
    return manager.get();
}

std::string DnsManager::getDnsServers() const {
    static std::vector<std::string> prebuiltDns = {
        "1.1.1.1",
        "1.0.0.1",
        "8.8.8.8",
        "8.8.4.4"
    };

    return boost::algorithm::join(prebuiltDns, ",");
}

}