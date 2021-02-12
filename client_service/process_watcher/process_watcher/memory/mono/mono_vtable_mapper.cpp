#include "process_watcher/memory/mono/mono_vtable_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONO_VTABLE_KLASS_OFFSET = 0;
constexpr uint32_t MONO_VTABLE_FLAGS_OFFSET = 28;
constexpr uint32_t MONO_VTABLE_VTABLE_OFFSET = 40;
constexpr uint8_t MONO_VTABLE_STATIC_FIELD_BITFLAG = 0b100;

}

MonoVTableMapper::MonoVTableMapper(MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr, int32_t vtableSize, int32_t domainId):
    _image(image),
    _memory(memory),
    _ptr(ptr),
    _vtableSize(vtableSize),
    _domainId(domainId) {

    uint32_t klassPtr = 0;
    _memory->readProcessMemory(&klassPtr, _ptr + MONO_VTABLE_KLASS_OFFSET);
    _klass = _image->loadClassFromPtr(klassPtr);

    uint8_t flags = 0;
    _memory->readProcessMemory(&flags, _ptr + MONO_VTABLE_FLAGS_OFFSET);
    _hasStaticFields = flags & MONO_VTABLE_STATIC_FIELD_BITFLAG;
}

uintptr_t MonoVTableMapper::getStaticDataPtr() const {
    if (!_hasStaticFields) {
        return 0;
    }

    uint32_t dataPtr = 0;
    _memory->readProcessMemory(&dataPtr, static_cast<uintptr_t>(_ptr + MONO_VTABLE_VTABLE_OFFSET + _vtableSize * 4));
    return static_cast<uintptr_t>(dataPtr);
}

}