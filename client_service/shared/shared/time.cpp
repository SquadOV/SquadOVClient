#include "shared/time/ntp_client.h"
#include "shared/time.h"

#include <date/tz.h>
#include <sstream>
#include <iomanip>

namespace shared {
namespace {

const std::chrono::seconds FUTURE_TIME_THRESHOLD = std::chrono::seconds(30);

}

TimePoint zeroTime() {
    return std::chrono::system_clock::from_time_t(0);
}

TimePoint nowUtc() {
    return shared::time::NTPClient::singleton()->now();
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

int64_t timeToUnixMs(const TimePoint& tm) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tm.time_since_epoch()).count();
}

TimePoint unixMsToTime(int64_t tm) {
    return shared::TimePoint(std::chrono::milliseconds(tm));
}

std::string timeToStr(const TimePoint& tm) {
    return date::format("%F %H:%M:%S", tm);
}

std::string timeToIso(const TimePoint& tm) {
    return date::format("%FT%TZ", tm);
}

TimePoint isoStrToTime(const std::string& dt) {
    return strToTime(dt, "%FT%TZ");
}

std::string fnameTimeToStr(const TimePoint& tm) {
    return date::format("%F-%H.%M.%S", tm);
}

bool isTimeValid(const TimePoint& tm) {
    return tm != TimePoint::max();
}

bool isTimeInFuture(const TimePoint& tm) {
    return tm > (shared::nowUtc() + FUTURE_TIME_THRESHOLD);
}

}