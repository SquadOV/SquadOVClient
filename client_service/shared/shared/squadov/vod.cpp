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
    };
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