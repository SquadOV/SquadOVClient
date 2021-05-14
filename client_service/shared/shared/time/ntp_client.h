#pragma once

#include "shared/time.h"

#include <cstdint>
#include <shared_mutex>
#include <thread>

namespace shared::time {

class NTPClient {
public:
    static NTPClient* singleton();
    ~NTPClient();

    void initialize();
    void enable(bool enabled, bool doTick = false);

    shared::TimePoint now() const;
    shared::TimePoint adjustTime(const shared::TimePoint& tm) const;
    shared::TimePoint revertTime(const shared::TimePoint& tm) const;
private:
    void tick();
    shared::TimePoint clientNow() const;
    int64_t offsetToServer(const std::string& server) const;

    mutable std::shared_mutex _offsetMutex;
    int64_t _offsetMs = 0;

    bool _enabled = true;
    bool _running = false;
    std::thread _tickThread;
};

}