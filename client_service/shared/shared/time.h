#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <date/date.h>

namespace shared {

using TimePoint = date::sys_time<std::chrono::system_clock::duration>;

TimePoint nowUtc();
TimePoint strToTime(const std::string& dt, const std::string& format);
std::string timeToStr(const TimePoint& tm);

}