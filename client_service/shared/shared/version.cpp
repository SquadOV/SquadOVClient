#include "shared/version.h"
#include <boost/algorithm/string.hpp>
#include <vector>

namespace shared {

VersionInfo parseVersionInfo(const std::string& vers) {
    VersionInfo info;

    std::vector<std::string> parts;
    boost::split(parts, vers, boost::is_any_of("."));

    if (parts.size() > 0) {
        info.major = std::stoi(parts[0]);
    }

    if (parts.size() > 1) {
        info.minor = std::stoi(parts[1]);
    }

    if (parts.size() > 2) {
        info.patch = std::stoi(parts[2]);
    }

    if (parts.size() > 3) {
        info.extra = std::stoi(parts[3]);
    }

    return info;
}

bool operator<(const VersionInfo& a, const VersionInfo& b) {
    return a.major < b.major && a.minor < b.minor && a.patch < b.patch && a.extra < b.extra;
}

}