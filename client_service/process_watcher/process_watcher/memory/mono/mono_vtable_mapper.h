#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include <memory>
#include <stdint.h>

namespace process_watcher::memory::mono {

// Maps a MonoVTable into our memory space.
// struct MonoVTable {
// 	(+0) MonoClass  *klass;
// 	 /*
// 	 * According to comments in gc_gcj.h, this should be the second word in
// 	 * the vtable.
// 	 */
// 	(+4) MonoGCDescriptor gc_descr;
// 	(+8) MonoDomain *domain;  /* each object/vtable belongs to exactly one domain */
//  (+12)       gpointer    type; /* System.Type type for klass */
// 	(+16) guint8     *interface_bitmap;
// 	(+20) guint32     max_interface_id;
// 	(+24) guint8      rank;
// 	/* Keep this a guint8, the jit depends on it */
// 	(+25) guint8      initialized; /* cctor has been run */
// 	(+28) guint remote          : 1; /* class is remotely activated */
// 	guint init_failed     : 1; /* cctor execution failed */
// 	guint has_static_fields : 1; /* pointer to the data stored at the end of the vtable array */
// 	guint gc_bits         : MONO_VTABLE_AVAILABLE_GC_BITS; /* Those bits are reserved for the usaged of the GC */
// 
// 	(+32) guint32     imt_collisions_bitmap;
// 	(+36) MonoRuntimeGenericContext *runtime_generic_context;
// 	/* do not add any fields after vtable, the structure is dynamically extended */
// 	/* vtable contains function pointers to methods or their trampolines, at the
// 	 end there may be a slot containing the pointer to the static fields */
// 	(+40) gpointer    vtable [MONO_ZERO_LEN_ARRAY];	
// };
class MonoVTableMapper {
public:
    MonoVTableMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr, int32_t vtableSize, int32_t domainId);

    uintptr_t ptr() const { return _ptr; }
    const class MonoClassMapper* klass() const { return _klass; }
    int32_t domainId() const { return _domainId; }

    uintptr_t getStaticDataPtr() const;
private:
    class MonoImageMapper* _image;
    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr = 0;
    int32_t _vtableSize;
    int32_t _domainId;

    bool _hasStaticFields = false;
    const class MonoClassMapper* _klass;
};

using MonoVTableMapperPtr = std::unique_ptr<MonoVTableMapper>;

}