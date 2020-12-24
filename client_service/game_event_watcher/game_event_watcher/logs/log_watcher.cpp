#include "game_event_watcher/logs/log_watcher.h"

#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/filesystem/utility.h"

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#include <WinBase.h>
#endif

#include <chrono>
#include <fstream>
#include <malloc.h>
#include <memory>
#include <iostream>
#include <locale>
#include <codecvt>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace game_event_watcher {

LogWatcher::LogWatcher(const fs::path& path, const LogChangeCallback& cb, const shared::TimePoint& timeThreshold, bool waitForNewFile, bool immediatelyGoToEnd):
    _path(path),
    _cb(cb),
    _timeThreshold(timeThreshold),
    _changeThread(&LogWatcher::watchWorker, this),
    _waitForNewFile(waitForNewFile),
    _immediatelyGoToEnd(immediatelyGoToEnd) {
}

LogWatcher::~LogWatcher() {
    _isFinished = true;
    _changeThread.join();
}

void LogWatcher::watchWorker() {
    const bool isCompletelyNewFile = !fs::exists(_path);
    if (_waitForNewFile || isCompletelyNewFile) {
        while (!_isFinished) {
            if (fs::exists(_path)) {
                const auto lastWriteTime = shared::filesystem::timeOfLastFileWrite(_path);
                if (lastWriteTime > _timeThreshold) {
                    break;
                }
            }
            std::this_thread::sleep_for(100ms);
        }
    }

    if (_isFinished) {
        LOG_INFO("Exiting out of log watcher before log was found: " << _path.string() << std::endl);
        // An early out here just in case we want to stop while still waiting for the log.
        return;
    }

    LOG_INFO("Found Log File: " << _path << std::endl);
    // Open file for reading via the C++ STDLIB.
    std::ifstream logStream;
    LogLinesDelta lineBuffer;

    // Open the file (if necessary) for the OS-specific way of watching for changes.
#ifdef _WIN32
    const fs::path parentDirectory = _path.parent_path();
    const std::wstring parentDirectoryFname = parentDirectory.native();

    HANDLE hDir = CreateFileW(     
        parentDirectoryFname.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );
#else
        THROW_ERROR("Unsupported OS for LogWatcher.");
#endif

    
    // Create another thread to open up the file every once in awhile.
    // Not sure why this is needed but some programs (e.g. Riot's client) won't actually flush
    // to the file unless this happens. This thread is also responsible for making sure
    // the call to ReadDirectoryChangesW gets cancelled.
    std::thread pollThread([this, hDir](){
        while (!_isFinished) {
            if (fs::exists(_path)) {
                try {
                    std::ifstream tmp(_path);
                    tmp.seekg(0, tmp.end);
                } catch (std::exception& ex) {
                    LOG_WARNING("Failed to poll log file: " << _path << " -- " << ex.what() << std::endl);
                }
            }
            std::this_thread::sleep_for(1s);
        }
        CancelIoEx(hDir, nullptr);
    });

    auto previousFilesize = std::filesystem::file_size(_path);

    while (!_isFinished) {
        logStream.clear();

        const auto currentFilesize = std::filesystem::file_size(_path);
        bool hasReset = false;
        if (currentFilesize < previousFilesize && logStream.is_open()) {
            // This happens in the case where we're watching a file which will have its content deleted and overwritten.
            // In this case the only thing we can really do is to reopen the file and start from the beginning
            logStream.close();
            hasReset = true;
        }

        // Read all available changes if the file is opened and exists.    
        // Totally OK if the log file isn't open yet but it doesn't exit or whatever.
        if (fs::exists(_path) && !logStream.is_open()) {
            // We don't want to set the std::ios_base::ate flag if we detected the contents of the file were wiped as we actually
            // do want to read in what was already written into the file.
            LOG_INFO("Open Log File for Reading: " << _path << " " << _immediatelyGoToEnd << " " << hasReset << " " << isCompletelyNewFile << std::endl);
            logStream.open(_path, (_immediatelyGoToEnd && !hasReset && !isCompletelyNewFile) ? std::ios_base::ate : std::ios_base::in);
            if (!logStream.is_open()) {
                LOG_WARNING("\tFailed to open log file: " << _path << std::endl);
            } else {
                LOG_INFO("\tSuccessfully opened log file." << std::endl);
            }
        } 
        previousFilesize = currentFilesize;
        
        try {
            std::string line;
            while (std::getline(logStream, line)) {
                if (_batchingEnabled) {
                    lineBuffer.push_back(line);
                } else {
                    _cb({line});
                }
            }

            if (_batchingEnabled) {
                // Let callback parse changes.
                _cb(lineBuffer);
                lineBuffer.clear();
            }
        } catch (std::exception& ex) {
            LOG_WARNING(
                "Failed to parse log line(s): " << ex.what() << std::endl
                    << "\tErrno: " << strerror(errno) << std::endl
                    << "\tWindows Error: " << shared::errors::getWin32ErrorAsString() << std::endl
                    << "\tGood: " << logStream.good() << std::endl
                    << "\tEOF: " << logStream.eof() << std::endl
                    << "\tFail: " << logStream.fail() << std::endl
                    << "\tBad: " << logStream.bad() << std::endl
            ); 
        }

        // Wait until the file changes again.
#ifdef _WIN32
        try {
            // Look for a change to log file in the directory that we want to watch.
            constexpr auto bufferNumBytes = 16384;
            std::unique_ptr<BYTE, decltype(&_aligned_free)> buffer((BYTE*)_aligned_malloc(bufferNumBytes, sizeof(BYTE)), _aligned_free);

            bool foundChanges = false;
            while (!foundChanges && !_isFinished) {  
                DWORD numBytes;
                BOOL ok = ReadDirectoryChangesW(
                    hDir,
                    buffer.get(),
                    bufferNumBytes,
                    FALSE, // Shouldn't need to watch subtree since the log file should be a direct descendant.
                    FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
                    &numBytes,
                    NULL,
                    NULL
                );

                // If numBytes == 0 then the system has too much info to give us...uhhh. TBD.
                if (!ok || numBytes == 0) {
                    // For whatever reason we failed...try again later.
                    std::this_thread::sleep_for(500ms);
                    continue;
                }

                BYTE* rawNotif = buffer.get();
                while (!!rawNotif && !_isFinished) {
                    FILE_NOTIFY_INFORMATION* notif = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(rawNotif);

                    // Ensure that the file that was changed is the file we care about.
                    const std::wstring wNotifFname(notif->FileName, notif->FileNameLength / sizeof(wchar_t));

                    if (notif->Action == FILE_ACTION_MODIFIED && wNotifFname == _path.filename().native()) {
                        foundChanges = true;
                        break;
                    }

                    if (notif->NextEntryOffset == 0) {
                        break;
                    }
                    rawNotif += notif->NextEntryOffset;
                }

                if (!foundChanges) {
                    // If we get here we didn't receive a notification about the file in question.
                    std::this_thread::sleep_for(10ms);
                }
            }
        } catch (std::exception& ex) {
            LOG_WARNING("Exception in waiting for directory change: " << ex.what() << std::endl);
        }
#endif
    }
    pollThread.join();
}

void LogWatcher::wait() {
    _changeThread.join();
}

}