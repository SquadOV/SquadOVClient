#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/mono/types.h"

namespace process_watcher::memory::mono {

class MonoObjectMapper {
public:
    MonoObjectMapper(
        class MonoImageMapper* image,
        const process_watcher::memory::ModuleMemoryMapperSPtr& memory,
        uintptr_t ptr,
        int32_t domainId,
        const class MonoClassMapper* klass = nullptr
    );

    uintptr_t ptr() const { return _ptr; }
    const process_watcher::memory::ModuleMemoryMapperSPtr& memory() const { return _memory; }
    MonoImageMapper* image() const { return _image; }
    const class MonoClassMapper* klass() const { return _klass; }
    int32_t domainId() const { return _domainId; }

    // Convenience functions to obtain field values for this specific object.
    DynamicMonoType get(const std::string& fieldName) const;
    DynamicMonoType superGet(const std::string& parentName, const std::string& fieldName) const;
    const class MonoTypeMapper* fieldType(const std::string& fieldName) const;
    const class MonoClassFieldMapper& field(const std::string& fieldName) const;
    const class MonoClassFieldMapper& superfield(const std::string& parentName, const std::string& fieldName) const;
    bool hasVtable() const { return !!_vtable; }
private:
    class MonoImageMapper* _image;
    const class MonoVTableMapper* _vtable;
    const class MonoClassMapper* _klass;
    int32_t _domainId;

    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr;
};


}