#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <date/date.h>

namespace shared {

using TimePoint = date::sys_time<std::chrono::system_clock::duration>;
using LocalTimePoint = date::local_seconds;

TimePoint zeroTime();
TimePoint nowUtc();
TimePoint strToTime(const std::string& dt, const std::string& format = "%F %T");
LocalTimePoint strToLocalTime(const std::string& dt, const std::string& format = "%F %T");

std::string timeToStr(const TimePoint& tm);
std::string timeToIso(const TimePoint& tm);

template<typename T>
std::string timeToDateString(const T& tm) {
    return date::format("%F", tm);
}

// A version of timeToStr that creates a date string that's more compatible with filenames.
std::string fnameTimeToStr(const TimePoint& tm);
bool isTimeValid(const TimePoint& tm);

template<typename T>
TimePoint convertTime(const T& tm) {
    return std::chrono::time_point_cast<TimePoint::duration>(tm - T::clock::now()
              + TimePoint::clock::now());
}

}