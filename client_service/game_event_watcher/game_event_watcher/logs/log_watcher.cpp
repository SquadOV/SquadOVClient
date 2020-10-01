#include "game_event_watcher/logs/log_watcher.h"

#include "shared/errors/error.h"
#include "shared/log/log.h"

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

LogWatcher::LogWatcher(const fs::path& path, const LogChangeCallback& cb, bool waitForNewFile):
    _path(path),
    _cb(cb),
    _changeThread(&LogWatcher::watchWorker, this),
    _waitForNewFile(waitForNewFile) {
}

LogWatcher::~LogWatcher() {
    _isFinished = true;
    _changeThread.join();
}

void LogWatcher::watchWorker() {
    if (_waitForNewFile || !fs::exists(_path.string())) {
        while (true) {
            const auto now = fs::file_time_type::clock::now();
            const auto lastWriteTime = fs::last_write_time(_path);
            const auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - lastWriteTime);
            const auto maxDiff = std::chrono::seconds(30);

            // The log file should have be written maxDiff of the current time OR the log file should just be newer.
            if (diff < maxDiff) {
                break;
            }

            LOG_INFO("\tDeferring log watcher due to a diff of " << diff.count() << " seconds." << std::endl);
            std::this_thread::sleep_for(1s);
            LOG_INFO("\t\tRestart watching: " << _path.string() << std::endl);
        }
    }

    // Open file for reading via the C++ STDLIB.
    std::ifstream logStream(_path.string());
    if (!logStream.is_open()) {
        THROW_ERROR("Failed to open log.");
    }

    // Create another thread to open up the file every once in awhile.
    // Not sure why this is needed but some programs (e.g. Riot's client) won't actually flush
    // to the file unless this happens.
    std::thread pollThread([this](){
        while (!_isFinished) {
            std::ifstream tmp(_path.string());
            tmp.seekg(0, tmp.end);
            std::this_thread::sleep_for(1s);
        }
    });

    LogLinesDelta lineBuffer;

    // Open the file (if necessary) for the OS-specific way of watching for changes.
#ifdef _WIN32
    const fs::path parentDirectory = _path.parent_path();
    const std::string parentDirectoryFname = parentDirectory.string();

    HANDLE hDir = CreateFile(     
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
    while (!_isFinished) {
        logStream.clear();
        // Read all available changes.
        std::string line;
        while (std::getline(logStream, line)) {
            lineBuffer.push_back(line);
        }

        // Let callback parse changes.
        _cb(lineBuffer);
        lineBuffer.clear();

        // Wait until the file changes again.
#ifdef _WIN32
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

                if (notif->Action == FILE_ACTION_MODIFIED) {
                    // Ensure that the file that was changed is the file we care about.
                    const std::wstring wNotifFname(notif->FileName, notif->FileNameLength / sizeof(wchar_t));

                    // This conversion is deprecated in C++17 but I can't seem to find a good alternative.
#pragma warning(disable: 4996)
                    const std::string notifName = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(wNotifFname);
                    if (notifName == _path.filename().string()) {
                        foundChanges = true;
                        break;
                    }
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
#endif
    }
    pollThread.join();
}

}