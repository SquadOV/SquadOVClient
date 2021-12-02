#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <ostream>

namespace service::speed_check {

enum class SpeedCheckManagerType {
    // Only S3 implemented right now
    // FileSystem,
    // GCS,
    S3,
};

struct SpeedCheckDestination {
    std::string url;
    std::string bucket;
    std::string session;
    SpeedCheckManagerType loc;

    static SpeedCheckDestination local(const std::filesystem::path& path);
    static SpeedCheckDestination fromJson(const nlohmann::json& obj);
};

}

std::ostream& operator<<(std::ostream& os, const service::speed_check::SpeedCheckDestination& x);