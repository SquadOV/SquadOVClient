#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

namespace process_watcher::memory::mono {

MonoObjectMapper::MonoObjectMapper(MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr, int32_t domainId):
    _image(image),
    _domainId(domainId),
    _memory(memory),
    _ptr(ptr) {

    uint32_t vtablePtr = 0;
    _memory->readProcessMemory(&vtablePtr, _ptr);
    _vtable = _image->loadVTableFromPtr(vtablePtr, _domainId);
    _klass = _vtable->klass();
}

DynamicMonoType MonoObjectMapper::get(const std::string& fieldName) const {
    const auto& field = _klass->field(fieldName);
    return field.get(this, _vtable->domainId());
}

const class MonoTypeMapper* MonoObjectMapper::fieldType(const std::string& fieldName) const {
    const auto& field = _klass->field(fieldName);
    return field.type();
}

}