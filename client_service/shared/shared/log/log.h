#pragma once
#pragma warning(disable: 4996)

#include "shared/time.h"

#include <atomic>
#include <deque>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <memory>
#include <mutex>
#include <optional>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace shared::log {

enum class LogType {
    Debug,
    Info,
    Warning,
    Error
};

class LogItem {
public:
    LogItem(LogType type, const std::string& data):
        _type(type),
        _data(data) {}
    
    LogType type() const { return _type; }
    std::string data() const { return _data; }
    std::string currentLogLevel() const;
    std::string sentryLevel() const;

private:
    LogType _type;
    std::string _data;
};

class LogItemBuilder {
public:
    using StdStreamManip = std::ostream&(*)(std::ostream&);
    using StdIoManip = std::ios_base&(*)(std::ios_base&);

    template<typename T>
    LogItemBuilder& operator<<(const T& val) {
        _data << val;
        return *this;
    }

    template<>
    LogItemBuilder& operator<<(const std::wstring& t) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        *this << conv.to_bytes(t);
        return *this;
    }

    template<>
    LogItemBuilder& operator<<(const std::filesystem::path& p) {
        *this << p.native();
        return *this;
    }

    template<>
    LogItemBuilder& operator<<(const shared::TimePoint& t) {
        *this << shared::timeToStr(t);
        return *this;
    }

    template<>
    LogItemBuilder& operator<<(const LogType& t) {
        _type = t;
        return *this;
    }

    LogItemBuilder& operator<<(StdStreamManip manip) {
        _data << manip;
        return *this;
    }

    LogItemBuilder& operator<<(StdIoManip manip) {
        _data << manip;
        return *this;
    }

    LogItem finish() const {
        return LogItem{_type, _data.str()};
    }

private:
    LogType _type;
    std::ostringstream _data;
};

class Log {
public:
    static void initializeGlobalLogger(const std::string& fname);
    static Log* singleton();

    explicit Log(const std::string& fname);
    ~Log();

    void setThreshold(LogType t) { _typeThreshold = t; }
    void enableSentry() { _sentryEnabled = true; }

    Log& operator<<(const LogItem& t) {
        {
            std::lock_guard<std::mutex> guard(_queueMutex);
            _queue.push_back(t);
        }
        _queueCv.notify_all();
        return *this;
    }

    void flush();

private:
    static std::unique_ptr<Log> _singleton;

    std::string currentTimeLog() const;
    bool canLogPass(const LogItem& t) const;

    LogType _typeThreshold = LogType::Info;
    std::ofstream _outLog;

    std::condition_variable _queueCv; 
    std::mutex _queueMutex;
    std::deque<LogItem> _queue;
    std::thread _queueThread;
    bool _running = true;
    std::atomic<bool> _sentryEnabled = false;
    void queueWorker();
};

std::filesystem::path generateMinidump(EXCEPTION_POINTERS* ex);

}

#define LOG_DEBUG(x) *shared::log::Log::singleton() << (shared::log::LogItemBuilder() << shared::log::LogType::Debug << x).finish();
#define LOG_INFO(x) *shared::log::Log::singleton() << (shared::log::LogItemBuilder() << shared::log::LogType::Info << x).finish();
#define LOG_WARNING(x) *shared::log::Log::singleton() << (shared::log::LogItemBuilder() << shared::log::LogType::Warning << x).finish();
#define LOG_ERROR(x) *shared::log::Log::singleton() << (shared::log::LogItemBuilder() << shared::log::LogType::Error << x).finish();
#define LOG_FLUSH() shared::log::Log::singleton()->flush();