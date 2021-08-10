#include "hardware/os.h"
#include "shared/log/log.h"
#ifdef _WIN32
#include <Windows.h>
#include <VersionHelpers.h>
#endif

namespace service::hardware {

OperatingSystem getOS() {
    OperatingSystem os;
#ifdef _WIN32
    os.name = "Windows";

    if (IsWindows10OrGreater()) {
        os.majorVersion = "10";
    } else if (IsWindows8Point1OrGreater()) {
        os.majorVersion = "8.1";
    } else if (IsWindows8OrGreater()) {
        os.majorVersion = "8";
    } else {
        os.majorVersion = "7";
    }

    HKEY key;
    auto res = RegOpenKeyExW(HKEY_LOCAL_MACHINE , L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &key);
    if (res != ERROR_SUCCESS) {
        LOG_WARNING("...Failed to get reg entry for Windows current version: " << res << std::endl);
        os.minorVersion = "Unknown";
        os.edition = "Unknown";
    } else {
        {
            char version[1024];
            DWORD count = 1024;
            DWORD dwType = REG_SZ;
            res = RegGetValueA(key, "", "DisplayVersion", RRF_RT_REG_SZ, &dwType, version, &count);
            if (res != ERROR_SUCCESS) {
                LOG_WARNING("...Failed to display version for Windows: " << res << std::endl);
                os.minorVersion = "Unknown";
            } else {
                os.minorVersion = std::string(version);
            }
        }

        {
            char edition[1024];
            DWORD count = 1024;
            DWORD dwType = REG_SZ;
            res = RegGetValueA(key, "", "EditionID", RRF_RT_REG_SZ, &dwType, edition, &count);
            if (res != ERROR_SUCCESS) {
                LOG_WARNING("...Failed to edition for Windows: " << res << std::endl);
                os.edition = "Unknown";
            } else {
                os.edition = std::string(edition);
            }
        }
    }
#endif
    return os;
}

nlohmann::json OperatingSystem::toJson() const {
    const nlohmann::json obj = {
        { "name", name },
        { "majorVersion", majorVersion },
        { "minorVersion", minorVersion },
        { "edition", edition }
    };
    return obj;
}

std::ostream& operator<<(std::ostream& os, const OperatingSystem& p) {
    os << "Name: " << p.name << std::endl
       << "Major: " << p.majorVersion << std::endl
       << "Minor: " << p.minorVersion << std::endl
       << "Edition: "<< p.edition;
    return os;
}

}