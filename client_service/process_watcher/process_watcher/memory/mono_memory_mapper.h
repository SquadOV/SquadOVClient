#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/pe_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

namespace process_watcher::memory {

// Pulls Mono-specific data out of memory using the already mapped
// module (ModuleMemoryMapper) and the already parsed PE information (PEMapper).
// Note that this is specific to mapping out the *Unity* fork of Mono.
class MonoMemoryMapper {
public:
    MonoMemoryMapper(const ModuleMemoryMapperSPtr& memory, const PEMapper& pe);

    friend std::ostream& operator<<(std::ostream& os, const MonoMemoryMapper& map);
private:
    uint32_t _rootDomainPtr = 0;
    mono::MonoImageMapperPtr _image;
};

std::ostream& operator<<(std::ostream& os, const MonoMemoryMapper& map);

}