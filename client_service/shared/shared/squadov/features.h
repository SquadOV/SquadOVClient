#pragma once

#include <stdint.h>

namespace shared::squadov {

struct FeatureFlags {
    int32_t maxRecordPixelY = 0;
    int32_t maxRecordFps = 0;
    int32_t maxBitrateKbps = 0;
    bool allowRecordUpload = false;
    bool allowWowCombatLogUpload = false;
    bool disableSentry = false;
    bool mandatoryWatermark = false;
    double watermarkMinSize;
};

inline std::ostream& operator<<(std::ostream& out, const FeatureFlags& flags) {
    out << "{ Max Y: " << flags.maxRecordPixelY << 
        ", Max FPS: " << flags.maxRecordFps <<
        ", Max Bitrate (kbps): " << flags.maxBitrateKbps <<
        ", Allow VOD Upload: " << flags.allowRecordUpload <<
        ", Allow WoW Combat Log Upload: " << flags.allowWowCombatLogUpload <<
        ", Disable Sentry: " << flags.disableSentry <<
        ", Mandatory Watermark: " << flags.mandatoryWatermark <<
        ", Watermark Min Size: " << flags.watermarkMinSize <<
    "}";
    return out;
}

}