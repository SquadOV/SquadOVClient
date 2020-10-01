#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

namespace shared::log {

enum class LogType {
    Info,
    Warning,
    Error
};

class Log {
public:
    explicit Log(const std::string& fname);
    ~Log();

    template<typename T>
    Log& operator<<(const T& val) {
        std::ostringstream str;
        if (_needPrefix) {
            str << "[" << currentLogLevel() << "]"
                << "[" << currentTimeLog() << "] ";
            _needPrefix = false;
        }
        str << val;

        _outLog << str.str();
        std::cout << str.str();
        return *this;
    } 

    template<>
    Log& operator<<(const LogType& t) {
        _currentType = t;
        return *this;
    }

    using StdStreamManip = std::ostream&(*)(std::ostream&);
    Log& operator<<(StdStreamManip manip) {
        if (manip == static_cast<StdStreamManip>(std::endl)) {
            _needPrefix = true;
        }

        _outLog << manip;
        std::cout << manip;
        return *this;
    }

private:
    std::string currentTimeLog() const;
    std::string currentLogLevel() const;

    LogType _currentType = LogType::Info;
    std::ofstream _outLog;
    bool _needPrefix = true;
};

Log& getGlobalLogger();

}

#define LOG_INFO(x) shared::log::getGlobalLogger() << shared::log::LogType::Info << x;
#define LOG_WARNING(x) shared::log::getGlobalLogger() << shared::log::LogType::Warning << x;
#define LOG_ERROR(x) shared::log::getGlobalLogger() << shared::log::LogType::Error << x;