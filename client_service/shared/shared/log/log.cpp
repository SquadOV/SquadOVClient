#include "shared/log/log.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/time.h"

#include <algorithm>
#include <vector>

#ifdef _WIN32
#include <DbgHelp.h>
#include <processthreadsapi.h>
#endif

namespace fs = std::filesystem;

namespace shared::log {
namespace {

constexpr auto maxLogsToKeep = 10;

}

std::unique_ptr<Log> Log::_singleton;

void Log::initializeGlobalLogger(const std::string& fname) {
    if (_singleton) {
        return;
    }
    _singleton = std::make_unique<Log>(fname);
}

Log* Log::singleton() {
    if (!_singleton) {
        throw std::runtime_error("Logging is not initialized.");
    }
    return _singleton.get();
}

Log::Log(const std::string& fname) {
    // Make sure the log directory exists.
    const auto logDir = shared::filesystem::getSquadOvServiceLogFolder();
    if (!fs::exists(logDir)) { 
        fs::create_directories(logDir);
    }

    // Try to create a new log file of the given name.
    // If the name already exists then backup the old file and then
    // create the new log file.
    const auto logFile = logDir / fs::path(fname);
    if (fs::exists(logFile)) {
        fs::rename(logFile, shared::filesystem::generateTimestampBackupFileName(logFile));
    }

    // Ensure that we don't keep too many backup logs. Only keep the most recent logs.
    auto dirIter = fs::directory_iterator(logDir);
    std::vector<fs::path> logPaths(fs::begin(dirIter), fs::end(dirIter));
    shared::filesystem::pruneFilesystemPaths(logPaths, maxLogsToKeep);

    // Create the output file. Make sure we we do this after the previous cleanup operation
    // so that we don't accidentally wipe the newest log.
    _outLog.open(logFile);
    if (!_outLog.is_open()) {
        // The only place where it's ok to throw a raw exception instead of going through the standard error path.
        throw std::runtime_error("Failed to open log file.");
        return;
    }

    // Start a thread to write to console/file from the queue.
    _queueThread = std::thread(std::bind(&Log::queueWorker, this));
}

void Log::flush() {
    std::unique_lock<std::mutex> lock(_queueMutex);
    if (!_queueCv.wait_for(lock, std::chrono::milliseconds(100), [this](){ return !_queue.empty(); })) {
        return;
    }

    while (!_queue.empty()) {
        const auto& item = _queue.front();
        
        if (canLogPass(item)) {
            std::ostringstream str;
            str << "[" << item.currentLogLevel() << "]"
                << "[" << currentTimeLog() << "] ";
            _outLog << str.str() << item.data() << std::flush;
            std::cout << str.str() << item.data() << std::flush;
        }

        _queue.pop_front();
    }
}

void Log::queueWorker() {
    while (_running) {
        flush();
    }
}

Log::~Log() {
    _running = false;
    if (_queueThread.joinable()) {
        _queueThread.join();
    }
    if (_outLog.is_open()) {
        _outLog.close();
    }
}

std::string Log::currentTimeLog() const {
    return shared::timeToStr(shared::nowUtc());
}

std::string LogItem::currentLogLevel() const {
    switch (_type) {
        case LogType::Info:
            return "Info";
        case LogType::Warning:
            return "Warning";
        case LogType::Error:
            return "Error";
        case LogType::Debug:
            return "Debug";
    }
    return "";
}

bool Log::canLogPass(const LogItem& t) const {
    return (static_cast<int>(t.type()) >= static_cast<int>(_typeThreshold));
}

fs::path generateMinidump(EXCEPTION_POINTERS* ex) {
    const auto dumpDir = shared::filesystem::getSquadOvServiceDumpFolder();
    if (!fs::exists(dumpDir)) { 
        fs::create_directories(dumpDir);
    }

    auto dirIter = fs::directory_iterator(dumpDir);
    std::vector<fs::path> logPaths(fs::begin(dirIter), fs::end(dirIter));
    shared::filesystem::pruneFilesystemPaths(logPaths, maxLogsToKeep - 1);

    std::ostringstream dumpFname;
    dumpFname << "squadov_" << shared::fnameTimeToStr(shared::nowUtc()) << ".dmp";
    const fs::path dumpPath = dumpDir / fs::path(dumpFname.str());

    MINIDUMP_EXCEPTION_INFORMATION info;
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = ex;
    info.ClientPointers = true;

    HANDLE hFile = CreateFileW(
        dumpPath.native().c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        &info,
        nullptr,
        nullptr
    );

    CloseHandle(hFile);
    return dumpPath;
}

}