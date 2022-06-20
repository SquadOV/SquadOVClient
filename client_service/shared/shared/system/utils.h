#pragma once

#include <stdint.h>

namespace shared::system::utils {

struct TimePrecisionInitializer {
    TimePrecisionInitializer();
    ~TimePrecisionInitializer();
};

void preciseSleep(int64_t nanoseconds);

}