#pragma once

#include <stdint.h>

namespace shared::squadov {

struct FeatureFlags {
    int32_t maxRecordPixelY = 0;
    int32_t maxRecordFps = 0;
    bool allowRecordUpload = false;
};

}