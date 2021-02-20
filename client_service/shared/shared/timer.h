#pragma once
#include <chrono>
#include <string>

using TimerClock = std::chrono::high_resolution_clock;
using TimerClockPoint = TimerClock::time_point;

namespace shared {

class Timer {
public:
    Timer(const std::string& prefix);
    ~Timer();

    void tick(const std::string& prefix);
private:
    TimerClockPoint _start;
    TimerClockPoint _lastTick;
    std::string _prefix;
};

}