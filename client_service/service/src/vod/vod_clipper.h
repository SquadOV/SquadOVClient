#pragma once
#include <nlohmann/json.hpp>
#include <string>

#include "shared/squadov/vod.h"

namespace service::vod {

struct VodClipRequest {
    std::string task;
    std::string source;
    int64_t start;
    int64_t end;
    bool fullCopy = false;
    std::string inputFormat = "mp4";
    bool audio = true;

    static VodClipRequest fromJson(const nlohmann::json& obj);
};

struct VodClipResponse {
    std::string task;
    std::string path;
    bool success;
    shared::squadov::VodMetadata metadata;

    nlohmann::json toJson() const;
};

VodClipResponse vodClip(const VodClipRequest& request);

}