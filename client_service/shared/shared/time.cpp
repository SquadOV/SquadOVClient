#include "shared/time.h"

#include <sstream>
#include <iomanip>

namespace shared {

TimePoint zeroTime() {
    return std::chrono::system_clock::from_time_t(0);
}

TimePoint nowUtc() {
    return std::chrono::system_clock::now();
}

TimePoint strToTime(const std::string& dt, const std::string& format) {
    std::istringstream input(dt);
    TimePoint ret;
    input >> date::parse(format, ret);
    return ret;
}

LocalTimePoint strToLocalTime(const std::string& dt, const std::string& format) {
    std::istringstream input(dt);
    LocalTimePoint ret;
    input >> date::parse(format, ret);
    return ret;
}

std::string timeToStr(const TimePoint& tm) {
    return date::format("%F %H:%M:%S", tm);
}

std::string timeToIso(const TimePoint& tm) {
    return date::format("%FT%TZ", tm);
}

std::string fnameTimeToStr(const TimePoint& tm) {
    return date::format("%F-%H.%M.%S", tm);
}

bool isTimeValid(const TimePoint& tm) {
    return tm != TimePoint::max();
}

}