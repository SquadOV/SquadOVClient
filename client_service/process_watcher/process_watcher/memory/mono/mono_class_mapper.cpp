#include "process_watcher/memory/mono/mono_class_mapper.h"

#include <sstream>

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONO_CLASS_NAME_OFFSET = 44;
constexpr uint32_t MONO_CLASS_NAMESPACE_OFFSET = 48;

}

MonoClassMapper::MonoClassMapper(const process_watcher::memory::ModuleMemoryMapper& memory, uintptr_t ptr):
    _ptr(ptr) {
    memory.readProcessMemory(_name, _ptr + MONO_CLASS_NAME_OFFSET, true);
    memory.readProcessMemory(_namespace, _ptr + MONO_CLASS_NAMESPACE_OFFSET, true);
}

std::string MonoClassMapper::fullName() const {
    std::ostringstream nm;
    if (!_namespace.empty()) {
        nm << _namespace << ".";
    }
    nm << _name;
    return nm.str();
}

}