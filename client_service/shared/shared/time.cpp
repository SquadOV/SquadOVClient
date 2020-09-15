#include "shared/time.h"

#include <sstream>
#include <iomanip>

namespace shared {

TimePoint nowUtc() {
    return std::chrono::system_clock::now();
}

TimePoint strToTime(const std::string& dt, const std::string& format) {
    std::istringstream input(dt);
    TimePoint ret;
    input >> date::parse(format, ret);
    return ret;
}

std::string timeToStr(const TimePoint& tm) {
    return date::format("%F %T", tm);
}

}