#include "shared/audio/timing.h"

namespace shared::audio {

std::chrono::nanoseconds samplesToNsDiff(size_t hz, size_t samples) {
    return std::chrono::nanoseconds(static_cast<int64_t>(samples / static_cast<double>(hz) * 1e9));
}

}