#pragma once

#include <chrono>

namespace shared::audio {

template<typename R, typename P>
size_t timeDiffToNumSamples(size_t hz, const std::chrono::duration<R, P>& duration) {
    const size_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return static_cast<size_t>(ns * 1.e-9 * hz);
}

std::chrono::nanoseconds samplesToNsDiff(size_t hz, size_t samples);

}