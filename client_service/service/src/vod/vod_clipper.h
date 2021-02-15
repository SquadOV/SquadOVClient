#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace service::vod {

struct VodClipRequest {
    std::string task;
    std::string source;
    int64_t start;
    int64_t end;

    static VodClipRequest fromJson(const nlohmann::json& obj);
};

struct VodClipResponse {
    std::string task;
    std::string path;
    bool success;

    nlohmann::json toJson() const;
};

VodClipResponse vodClip(const VodClipRequest& request);

}