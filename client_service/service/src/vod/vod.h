#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <ostream>

namespace service::vod {

enum class VodManagerType {
    FileSystem,
    GCS,
    S3,
};

struct VodDestination {
    std::string url;
    std::string bucket;
    std::string session;
    VodManagerType loc;

    static VodDestination local(const std::filesystem::path& path);
    static VodDestination fromJson(const nlohmann::json& obj);
};

}

std::ostream& operator<<(std::ostream& os, const service::vod::VodDestination& x);