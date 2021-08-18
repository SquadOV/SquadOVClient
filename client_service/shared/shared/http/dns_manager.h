#pragma once

#include <memory>
#include <string>

namespace shared::http {

class DnsManager {
public:
    void enable(bool v) { _isActive = v; }

    bool isActive() const { return _isActive; }
    std::string getDnsServers() const;

private:
    bool _isActive = false;
};

using DnsManagerPtr = std::unique_ptr<DnsManager>;

DnsManager* getDnsManager();

}