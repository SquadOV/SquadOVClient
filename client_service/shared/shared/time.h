#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <date/date.h>
#include <date/tz.h>

namespace shared {

using TimePoint = date::sys_time<std::chrono::system_clock::duration>;
using LocalTimePoint = date::local_seconds;

TimePoint zeroTime();
TimePoint nowUtc();
TimePoint strToTime(const std::string& dt, const std::string& format = "%F %T");
TimePoint isoStrToTime(const std::string& dt);
LocalTimePoint strToLocalTime(const std::string& dt, const std::string& format = "%F %T");

int64_t timeToUnixMs(const TimePoint& tm);
TimePoint unixMsToTime(int64_t tm);
std::string timeToStr(const TimePoint& tm);
std::string timeToIso(const TimePoint& tm);

template<typename T>
std::string timeToDateString(const T& tm) {
    return date::format("%F", tm);
}

template<typename T>
std::string localTimeToString(const date::local_time<T>& t) {
    const auto dp = date::floor<date::days>(t);
    date::year_month_day ymd(dp);
    date::hh_mm_ss hms(date::floor<std::chrono::milliseconds>(t - dp));

    std::ostringstream ss;
    ss << ymd.year() << "-" << ymd.month() << "-" << ymd.day() << " " << hms;
    return ss.str();
}

// A version of timeToStr that creates a date string that's more compatible with filenames.
std::string fnameTimeToStr(const TimePoint& tm);
bool isTimeValid(const TimePoint& tm);
bool isTimeInFuture(const TimePoint& tm);

template<typename T>
TimePoint convertTime(const T& tm) {
    return std::chrono::time_point_cast<TimePoint::duration>(tm - T::clock::now()
              + TimePoint::clock::now());
}

}