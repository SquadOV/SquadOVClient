#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"

namespace process_watcher::memory::mono {

MonoObjectMapper::MonoObjectMapper(MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr, int32_t domainId, const class MonoClassMapper* klass):
    _image(image),
    _domainId(domainId),
    _memory(memory),
    _ptr(ptr) {

    if (klass) {
        _klass = klass;
        if (!klass->isValueType()) {
            _vtable = _image->loadVTableForClass(_klass, _domainId);
        } else {
            _vtable = nullptr;
        }
    } else {
        uint32_t vtablePtr = 0;
        _memory->readProcessMemory(&vtablePtr, _ptr);
        _vtable = _image->loadVTableFromPtr(vtablePtr, _domainId);
        _klass = _vtable->klass();
    }
}

DynamicMonoType MonoObjectMapper::get(const std::string& fieldName) const {
    const auto* field = this->field(fieldName);
    if (!field) {
        return DynamicMonoType{};
    }
    return field->get(this, _vtable ? _vtable->domainId() : 0);
}

DynamicMonoType MonoObjectMapper::superGet(const std::string& parentName, const std::string& fieldName) const {
    const auto* field = superfield(parentName, fieldName);
    if (!field) {
        return DynamicMonoType{};
    }
    return field->get(this, _vtable ? _vtable->domainId() : 0);
}

const class MonoTypeMapper* MonoObjectMapper::fieldType(const std::string& fieldName) const {
    const auto* field = _klass->field(fieldName);
    if (!field) {
        return nullptr;
    }
    return field->type();
}

const class MonoClassFieldMapper* MonoObjectMapper::field(const std::string& fieldName) const {
    return _klass->field(fieldName);
}

const class MonoClassFieldMapper* MonoObjectMapper::superfield(const std::string& parentName, const std::string& fieldName) const {
    const auto* klass = _klass->superClass(parentName);
    return klass->field(fieldName);
}

}