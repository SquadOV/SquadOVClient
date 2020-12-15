#include "shared/filesystem/utility.h"

namespace fs = std::filesystem;
namespace shared::filesystem {

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
    const auto lastWriteTime = fs::last_write_time(pth);
    return std::chrono::duration_cast<std::chrono::seconds>(now - lastWriteTime);
}

shared::TimePoint timeOfLastFileWrite(const std::filesystem::path& pth) {
    return shared::nowUtc() - secondsSinceLastFileWrite(pth);
}

}