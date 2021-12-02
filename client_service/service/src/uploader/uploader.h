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

struct UploadDestination {
    std::string url;
    std::string bucket;
    std::string session;
    UploadManagerType loc;

    static UploadDestination local(const std::filesystem::path& path);
    static UploadDestination fromJson(const nlohmann::json& obj);
};

}

std::ostream& operator<<(std::ostream& os, const service::uploader::UploadDestination& x);