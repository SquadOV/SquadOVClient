#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/mono/mono_type_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"

#include <memory>
#include <string>

namespace process_watcher::memory::mono {

class MonoClassMapper;
// Pulls data out of memory that represents a MonoClassField.
// struct _MonoClassField {
// 	/* Type of the field */
// 	MonoType        *type;
// 
// 	const char      *name;
// 
// 	/* Type where the field was defined */
// 	MonoClass       *parent;
// 
// 	/*
// 	 * Offset where this field is stored; if it is an instance
// 	 * field, it's the offset from the start of the object, if
// 	 * it's static, it's from the start of the memory chunk
// 	 * allocated for statics for the class.
// 	 * For special static fields, this is set to -1 during vtable construction.
// 	 */
// 	int              offset;
// };
class MonoClassFieldMapper {
public:
    MonoClassFieldMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr);
    const std::string& name() const { return _name; }

    DynamicMonoType getStatic(int32_t domainId) const;
    DynamicMonoType get(const class MonoObjectMapper* object, int32_t domainId) const;
    int32_t offset() const { return _classfield.offset; }

    const MonoTypeMapper* type() const { return _type; }

    friend std::ostream& operator<<(std::ostream& os, const MonoClassFieldMapper& map);
private:
    class MonoImageMapper* _image;
    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr = 0;
    struct RawMonoClassField {
        uint32_t typePtr;
        uint32_t namePtr;
        uint32_t parentPtr;
        int32_t offset;
    };
    RawMonoClassField _classfield;

    // Parsed out fields.
    std::string _name;
    const MonoClassMapper* _class;
    const MonoTypeMapper* _type;
};

std::ostream& operator<<(std::ostream& os, const MonoClassFieldMapper& map);
using MonoClassFieldMapperPtr = std::unique_ptr<MonoClassFieldMapper>;

}