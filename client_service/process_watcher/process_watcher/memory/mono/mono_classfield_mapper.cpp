#include "process_watcher/memory/mono/mono_classfield_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

namespace process_watcher::memory::mono {

MonoClassFieldMapper::MonoClassFieldMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr):
    _image(image),
    _ptr(ptr) {
    memory->readProcessMemory(&_classfield, ptr);
    memory->readProcessMemory(_name, _classfield.namePtr, false);
    _type = _image->loadTypeFromPtr(static_cast<uintptr_t>(_classfield.typePtr));
}

std::ostream& operator<<(std::ostream& os, const MonoClassFieldMapper& map) {
    os << "Field: " << map._name;
    os << std::endl << "Type: " << *map._type;
    return os;
}

}