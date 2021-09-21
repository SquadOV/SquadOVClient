#pragma once

#include <stdint.h>

namespace shared::squadov {

struct FeatureFlags {
    int32_t maxRecordPixelY = 0;
    int32_t maxRecordFps = 0;
    bool allowRecordUpload = false;
    bool allowWowCombatLogUpload = false;
    bool disableSentry = false;
};

inline std::ostream& operator<<(std::ostream& out, const FeatureFlags& flags) {
    out << "{ Max Y: " << flags.maxRecordPixelY << 
        ", Max FPS: " << flags.maxRecordFps <<
        ", Allow VOD Upload: " << flags.allowRecordUpload <<
        ", Allow WoW Combat Log Upload: " << flags.allowWowCombatLogUpload <<
        ", Disable Sentry: " << flags.disableSentry << "}";
    return out;
}

}