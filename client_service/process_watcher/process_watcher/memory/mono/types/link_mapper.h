#pragma once

#include "process_watcher/memory/mono/types/struct.h"
#include <stdint.h>

namespace process_watcher::memory::mono::types {

struct LinkMapper: public RawStruct {
    int32_t hashCode = 0;
    int32_t next = 0;
};

}