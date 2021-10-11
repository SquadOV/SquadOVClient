extern "C" {
#include <ares.h>
}

#include "shared/http/dns_manager.h"
#include "shared/log/log.h"

#include <boost/algorithm/string.hpp>

namespace shared::http {
namespace {

static const std::vector<std::string> PREBUILT_DNS = {
    "8.8.8.8",
    "8.8.4.4",
    "1.1.1.1",
    "1.0.0.1"
};

void aresCallback(void* arg, int status, int timeouts, hostent* ent) {
    DnsManager* manager = reinterpret_cast<DnsManager*>(arg);
    manager->onAresCallback(status, timeouts, ent);
}

}

DnsManager::DnsManager() {
    ares_library_init(ARES_LIB_INIT_ALL);
    sanityCheckDns();
}

DnsManager::~DnsManager() {
    ares_library_cleanup();
}

void DnsManager::sanityCheckDns() {
    // Need to do a sanity check here to make sure we can actually resolve DNS correctly
    // using our custom DNS servers before having the rest of the application using these servers.
    LOG_INFO("Sanity checking DNS..." << std::endl);

    ares_channel channel = nullptr;
    ares_options opts = { 0 };
    
    in_addr servers[4];
    for (int i = 0; i < 4; ++i) {
        ares_inet_pton(AF_INET, PREBUILT_DNS[i].c_str(), &servers[i]);
    }
    
    if (ares_init(&channel/*, &opts, ARES_OPT_SERVERS*/) != ARES_SUCCESS) {
        LOG_WARNING("...Failed to init C-ARES." << std::endl);
        return;
    }

    // Send the DNS query - cleanup with happen on callback.
    ares_gethostbyname(channel, "api.squadov.gg", AF_INET, aresCallback, this);

    // Start a thread that handles the event loop for C-ARES.
    _eventLoop = std::thread([this, channel](){
        int nfds, count;
        fd_set readers, writers;
        timeval tv, *tvp;

        while (_checkingSupport) {
            FD_ZERO(&readers);
            FD_ZERO(&writers);
            nfds = ares_fds(channel, &readers, &writers);
            if (!nfds) {
                _checkingSupport = false;
                break;
            }

            tvp = ares_timeout(channel, nullptr, &tv);
            count = select(nfds, &readers, &writers, nullptr, tvp);
            ares_process(channel, &readers, &writers);
        }
    });
}

void DnsManager::onAresCallback(int status, int timeouts, void* ent) {
    hostent* hent = reinterpret_cast<hostent*>(ent);
    if (status != ARES_SUCCESS) {
        LOG_WARNING("...Failed to lookup DNS: " << ares_strerror(status) << std::endl);
        _isDnsSupported = false;
    } else {
        LOG_INFO("...DNS Lookup succeeded for " << hent->h_name << std::endl);
        switch (hent->h_addrtype) {
            case AF_INET:
                LOG_INFO("...Found IPv4." << std::endl)
                break;
            case AF_INET6:
                LOG_INFO("...Found IPv6." << std::endl)
                break;
        }

        for (int i = 0; hent->h_addr_list[i] != NULL; ++i) {
            char buffer[INET6_ADDRSTRLEN];
            ares_inet_ntop(hent->h_addrtype, hent->h_addr_list[i], buffer, 46);
            LOG_INFO("\t IP: " << buffer << std::endl);
        }

        _isDnsSupported = true;
    }

    _checkingSupport = false;
}

DnsManager* getDnsManager() {
    static DnsManagerPtr manager = std::make_unique<DnsManager>();
    return manager.get();
}

std::string DnsManager::getDnsServers() const {
    return boost::algorithm::join(PREBUILT_DNS, ",");
}

}