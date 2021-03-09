#pragma once

#include <string>

namespace shared {

struct VersionInfo {
    int major = 0;
    int minor = 0;
    int patch = 0;
    int extra = 0;
};

VersionInfo parseVersionInfo(const std::string& vers);
bool operator<(const VersionInfo& a, const VersionInfo& b);

}