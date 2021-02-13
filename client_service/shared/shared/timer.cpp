#include "shared/timer.h"
#include "shared/log/log.h"

namespace shared {

Timer::Timer(const std::string& prefix):
    _prefix(prefix) {
    _start = TimerClock::now();
    _lastTick = _start;
    LOG_INFO("START " << _prefix << std::endl);
}

Timer::~Timer() {
    tick("END: " + _prefix);
}

void Timer::tick(const std::string& prefix) {
    const auto now = TimerClock::now();
    const auto elapsedStart = std::chrono::duration_cast<std::chrono::microseconds>(now - _start);
    const auto elapsedTick = std::chrono::duration_cast<std::chrono::microseconds>(now - _lastTick);

    LOG_INFO(
        prefix 
            << " [START " << elapsedStart.count() * 1.e-3 << "ms] [TICK " 
            << elapsedTick.count() * 1.e-3 << "ms]" << std::endl
    );

    _lastTick = now;
}

}