#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <ostream>

namespace service::uploader {

enum class UploadManagerType {
    FileSystem,
    GCS,
    S3,
};

// Not in love with this name, but whatever for now
enum class UploadPurpose {
    VOD,
    SpeedCheck,
};

struct UploadDestination {
    std::string url;
    std::string bucket;
    std::string session;
    UploadManagerType loc;
    UploadPurpose purpose;

    static UploadDestination local(const std::filesystem::path& path);
    static UploadDestination fromJson(const nlohmann::json& obj);
};

}

std::ostream& operator<<(std::ostream& os, const service::uploader::UploadDestination& x);