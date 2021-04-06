#include "shared/squadov/vod.h"
#include "shared/time.h"

namespace shared::squadov {

nlohmann::json VodAssociation::toJson() const {
    return {
        { "matchUuid", matchUuid },
        { "userUuid", userUuid },
        { "videoUuid", videoUuid },
        { "startTime", shared::timeToIso(startTime) },
        { "endTime", shared::timeToIso(endTime  ) },
        { "rawContainerFormat", rawContainerFormat },
        { "isClip" , isClip },
        { "isLocal", isLocal }
    };
}

VodAssociation VodAssociation::fromJson(const nlohmann::json& j) {
    VodAssociation v;
    v.matchUuid = j["matchUuid"].get<std::string>();
    v.userUuid = j["userUuid"].get<std::string>();
    v.videoUuid = j["videoUuid"].get<std::string>();
    v.startTime = shared::isoStrToTime(j["startTime"].get<std::string>());
    v.endTime = shared::isoStrToTime(j["endTime"].get<std::string>());
    v.rawContainerFormat = j["rawContainerFormat"].get<std::string>();
    v.isClip = j.value("isClip", false);
    v.isLocal = j.value("isLocal", false);
    return v;
}

nlohmann::json VodMetadata::toJson() const {
    return {
        { "videoUuid", videoUuid },
        { "resX", resX },
        { "resY", resY },
        { "minBitrate", minBitrate },
        { "avgBitrate", avgBitrate },
        { "maxBitrate", maxBitrate },
        { "id", id },
        { "fps", fps },
    };
}

}