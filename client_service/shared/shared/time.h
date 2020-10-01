#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <date/date.h>

namespace shared {

using TimePoint = date::sys_time<std::chrono::system_clock::duration>;

TimePoint nowUtc();
TimePoint strToTime(const std::string& dt, const std::string& format);
std::string timeToStr(const TimePoint& tm, bool withMs = false);
// A version of timeToStr that creates a date string that's more compatible with filenames.
std::string fnameTimeToStr(const TimePoint& tm);
bool isTimeValid(const TimePoint& tm);

template<typename T>
TimePoint convertTime(const T& tm) {
    return std::chrono::time_point_cast<TimePoint::duration>(tm - T::clock::now()
              + TimePoint::clock::now());
}

}