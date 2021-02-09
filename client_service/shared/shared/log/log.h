#pragma once
#pragma warning(disable: 4996)

#include <deque>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
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

class Log {
public:
    using StdStreamManip = std::ostream&(*)(std::ostream&);
    using StdIoManip = std::ios_base&(*)(std::ios_base&);

    struct LogItem {
        std::optional<LogType> type;
        std::optional<std::string> text;
        std::optional<StdStreamManip> stream;
        std::optional<StdIoManip> io;
        std::optional<bool> needPrefix;
    };

    explicit Log(const std::string& fname);
    ~Log();

    void setThreshold(LogType t) { _typeThreshold = t; }

    template<typename T>
    Log& operator<<(const T& val) {
        std::ostringstream str;
        str << val;
        LogItem item;
        item.text = str.str();
        *this << item;
        return *this;
    }

    template<>
    Log& operator<<(const std::wstring& t) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        *this << conv.to_bytes(t);
        return *this;
    }

    template<>
    Log& operator<<(const std::filesystem::path& p) {
        *this << p.native();
        return *this;
    }

    template<>
    Log& operator<<(const LogType& t) {
        LogItem item;
        item.type = t;
        *this << item;
        return *this;
    }

    Log& operator<<(StdStreamManip manip) {
        LogItem item;
        item.stream = manip;
        item.needPrefix = (manip == static_cast<StdStreamManip>(std::endl));
        *this << item;
        return *this;
    }

    
    Log& operator<<(StdIoManip manip) {
        if (!canLogPass()) {
            return *this;
        }
        
        _outLog << manip;
        std::cout << manip;
        return *this;
    }

    template<>
    Log& operator<<(const LogItem& t) {
        {
            std::lock_guard<std::mutex> guard(_queueMutex);
            _queue.push_back(t);
        }
        _queueCv.notify_all();
        return *this;
    }

private:
    std::string currentTimeLog() const;
    std::string currentLogLevel() const;
    bool canLogPass() const;

    LogType _currentType = LogType::Info;
    LogType _typeThreshold = LogType::Info;
    std::ofstream _outLog;
    bool _needPrefix = true;

    std::condition_variable _queueCv; 
    std::mutex _queueMutex;
    std::deque<LogItem> _queue;
    std::thread _queueThread;
    bool _running = true;
    void queueWorker();
};

Log& getGlobalLogger();
std::filesystem::path generateMinidump(EXCEPTION_POINTERS* ex);

}

#define LOG_DEBUG(x) shared::log::getGlobalLogger() << shared::log::LogType::Debug << x;
#define LOG_INFO(x) shared::log::getGlobalLogger() << shared::log::LogType::Info << x;
#define LOG_WARNING(x) shared::log::getGlobalLogger() << shared::log::LogType::Warning << x;
#define LOG_ERROR(x) shared::log::getGlobalLogger() << shared::log::LogType::Error << x;