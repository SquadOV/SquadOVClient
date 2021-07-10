#pragma warning(disable: 4996)

#include "shared/filesystem/utility.h"
#include "shared/errors/error.h"
#include <codecvt>
#include <fstream>
#include <iterator>

#ifdef _WIN32
#include <Windows.h>
#include <VersionHelpers.h>
#endif

namespace fs = std::filesystem;
namespace shared::filesystem {
namespace {

#ifdef _WIN32
fs::file_time_type::clock::time_point win32LastWriteTime(const std::filesystem::path& path) {
    const auto nativePath = path.native();

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(nativePath.c_str(), GetFileExInfoStandard, &data)) {
        THROW_ERROR("Failed to get file attributes: " << shared::errors::getWin32ErrorAsString());
        return fs::file_time_type::clock::now();
    }

    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&data.ftLastWriteTime, &st)) {
        THROW_ERROR("Failed to convert to system time: " << shared::errors::getWin32ErrorAsString());
        return fs::file_time_type::clock::now();
    }

    const auto sysTime = date::sys_days(date::year(st.wYear)/date::month(st.wMonth)/date::day(st.wDay))
        + std::chrono::hours(st.wHour) + std::chrono::minutes(st.wMinute) + std::chrono::seconds(st.wSecond) + std::chrono::milliseconds(st.wMilliseconds);
    return shared::convertClockTime<decltype(sysTime), fs::file_time_type>(sysTime);
}
#endif

}

std::filesystem::path getNewestFileInFolder(const std::filesystem::path& folder, const PathFilter& filter) {
    auto latestWriteTime = fs::file_time_type::min();
    fs::path retPath;

    for (const auto& entry : fs::recursive_directory_iterator(folder)) {
        const auto path = entry.path();
        if (filter && !filter(path)) {
            continue;
        }

        const auto lastWriteTime = fs::last_write_time(path);
        if (lastWriteTime > latestWriteTime) {
            latestWriteTime = lastWriteTime;
            retPath = path;
        }
    }

    return retPath;
}

std::chrono::seconds secondsSinceLastFileWrite(const std::filesystem::path& pth) {
    const auto now = fs::file_time_type::clock::now();
#ifdef _WIN32
    const auto lastWriteTime = IsWindows10OrGreater() ? fs::last_write_time(pth) : win32LastWriteTime(pth);
#endif
    return std::chrono::duration_cast<std::chrono::seconds>(now - lastWriteTime);
}

shared::TimePoint timeOfLastFileWrite(const std::filesystem::path& pth) {
    return shared::nowUtc() - secondsSinceLastFileWrite(pth);
}

std::string pathUtf8(const std::filesystem::path& path) {
    const auto npath = path.native();
#ifdef _WIN32
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(npath);
#else
    return npath;
#endif
}

std::filesystem::path generateTimestampBackupFileName(const std::filesystem::path& fname) {
    const auto lastWriteTime = shared::convertTime(fs::last_write_time(fname));
    auto backupFile = fname;

    auto newFname = backupFile.filename().stem();
    newFname += fs::path("_");
    newFname += fs::path(shared::fnameTimeToStr(lastWriteTime));
    newFname += backupFile.extension();

    backupFile.replace_filename(newFname);
    return backupFile;
}

void pruneFilesystemPaths(std::vector<std::filesystem::path>& paths, int maxKeep) {
    if (paths.size() > static_cast<size_t>(maxKeep)) {
        // Sort the files as oldest to newest so that the first X files are the X oldest files.
        // And we just want to delete X = N - maxLogsToKeep files where N is the total numbero
        // of log files.
        std::sort(paths.begin(), paths.end(), [](const fs::path& a, const fs::path& b){
            return fs::last_write_time(a) < fs::last_write_time(b);
        });

        const auto numToRemove = paths.size() - static_cast<size_t>(maxKeep);
        for (size_t i = 0; i < numToRemove; ++i) {
            fs::remove(paths[i]);
        }
    }
}

void readBinaryData(std::vector<char>& data, const std::filesystem::path& from) {
    std::ifstream stream(from, std::ios_base::binary);
    stream.unsetf(std::ios::skipws);

    stream.seekg(0, std::ios::end);
    const std::streampos fileEnd = stream.tellg();
    stream.seekg(0, std::ios::beg);

    data.reserve(fileEnd);
    data.insert(data.begin(), std::istream_iterator<char>(stream), std::istream_iterator<char>());
}

size_t getFolderSizeBytes(const std::filesystem::path& path) {
    if (!fs::exists(path)) {
        return 0;
    }

    size_t bytes = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        bytes += fs::file_size(entry);
    }
    return bytes;
}

}