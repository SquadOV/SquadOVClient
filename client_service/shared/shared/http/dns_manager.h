#pragma once

#include <memory>
#include <string>
#include <thread>

namespace shared::http {

class DnsManager {
public:
    DnsManager();
    ~DnsManager();

    // Probably better to have a private function with a friend function but meh...
    void onAresCallback(int status, int timeouts, void* ent);
private:
    void sanityCheckDns();

    bool _isDnsSupported = false;

    bool _checkingSupport = true;
    std::thread _eventLoop;
};

using DnsManagerPtr = std::unique_ptr<DnsManager>;

DnsManager* getDnsManager();

}