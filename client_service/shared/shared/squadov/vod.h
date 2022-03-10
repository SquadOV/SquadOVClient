#pragma once

#include "shared/time.h"
#include <filesystem>
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

namespace shared::squadov {

struct VodAssociation {
    std::string matchUuid;
    std::string userUuid;
    std::string videoUuid;
    shared::TimePoint startTime;
    shared::TimePoint endTime;
    std::string rawContainerFormat;
    bool isClip = false;
    bool isLocal = false;

    nlohmann::json toJson() const;
    static VodAssociation fromJson(const nlohmann::json& j);
};

struct VodMetadata {
    std::string videoUuid;
    int resX;
    int resY;

    int64_t minBitrate;
    int64_t avgBitrate;
    int64_t maxBitrate;

    int fps;
    std::string bucket;
    std::optional<std::string> sessionId;

    std::string id;
    nlohmann::json toJson() const;
};

std::optional<VodMetadata> generateVodMetadataFromFile(const std::filesystem::path& fname);

}