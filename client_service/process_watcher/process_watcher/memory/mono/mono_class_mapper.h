#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/mono/mono_classfield_mapper.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace process_watcher::memory::mono {

// Pulls data out of memory that represents a MonoClass.
// struct _MonoClass {
// 	/* element class for arrays and enum basetype for enums */
// 	(+0) MonoClass *element_class; 
// 	/* used for subtype checks */
// 	(+4) MonoClass *cast_class; 
// 
// 	/* for fast subtype checks */
// 	(+8) MonoClass **supertypes;
// 	(+12) guint16     idepth;
// 
// 	/* array dimension */
// 	(+14) guint8     rank;          
// 
// 	(+16) int        instance_size; /* object instance size */ --> I believe padding will come in before this.
// 
// 	(+20) guint inited          : 1;
// 
// 	/* A class contains static and non static data. Static data can be
// 	 * of the same type as the class itselfs, but it does not influence
// 	 * the instance size of the class. To avoid cyclic calls to 
// 	 * mono_class_init (from mono_class_instance_size ()) we first 
// 	 * initialise all non static fields. After that we set size_inited 
// 	 * to 1, because we know the instance size now. After that we 
// 	 * initialise all static fields.
// 	 */
// 
// 	/* ALL BITFIELDS SHOULD BE WRITTEN WHILE HOLDING THE LOADER LOCK */
// 	guint size_inited     : 1;
// 	guint valuetype       : 1; /* derives from System.ValueType */
// 	guint enumtype        : 1; /* derives from System.Enum */
// 	guint blittable       : 1; /* class is blittable */
// 	guint unicode         : 1; /* class uses unicode char when marshalled */
// 	guint wastypebuilder  : 1; /* class was created at runtime from a TypeBuilder */
// 	guint is_array_special_interface : 1; /* gtd or ginst of once of the magic interfaces that arrays implement */
// 
// 	/* next byte */
// 	(+21) guint8 min_align;
// 
// 	/* next byte */
// 	(+22) guint packing_size    : 4;
// 	guint ghcimpl         : 1; /* class has its own GetHashCode impl */ 
// 	guint has_finalize    : 1; /* class has its own Finalize impl */ 
// #ifndef DISABLE_REMOTING
// 	guint marshalbyref    : 1; /* class is a MarshalByRefObject */
// 	guint contextbound    : 1; /* class is a ContextBoundObject */
// #endif
// 	/* next byte */
// 	(+23) guint delegate        : 1; /* class is a Delegate */
// 	guint gc_descr_inited : 1; /* gc_descr is initialized */
// 	guint has_cctor       : 1; /* class has a cctor */
// 	guint has_references  : 1; /* it has GC-tracked references in the instance */
// 	guint has_static_refs : 1; /* it has static fields that are GC-tracked */
// 	guint no_special_static_fields : 1; /* has no thread/context static fields */
// 	/* directly or indirectly derives from ComImport attributed class.
// 	 * this means we need to create a proxy for instances of this class
// 	 * for COM Interop. set this flag on loading so all we need is a quick check
// 	 * during object creation rather than having to traverse supertypes
// 	 */
// 	guint is_com_object : 1; 
// 	guint nested_classes_inited : 1; /* Whenever nested_class is initialized */
// 
// 	/* next byte*/
// 	(+30) guint class_kind : 3; /* One of the values from MonoTypeKind */
// 	guint interfaces_inited : 1; /* interfaces is initialized */
// 	guint simd_type : 1; /* class is a simd intrinsic type */
// 	guint has_finalize_inited    : 1; /* has_finalize is initialized */
// 	guint fields_inited : 1; /* setup_fields () has finished */
// 	guint has_failure : 1; /* See mono_class_get_exception_data () for a MonoErrorBoxed with the details */
// 	guint has_weak_fields : 1; /* class has weak reference fields */
// 
// 	(+32) MonoClass  *parent; --> between class_kind and parent are 9 bits so I'm assuming it's going to pad out to 28 bytes
// 	(+36) MonoClass  *nested_in;
// 
//  (+40) MonoImage *image; --> I confirmed this number in disassembler. There's probably nesting shenanigans going on with the bitfields...
// 	(+44) const char *name;
// 	(+48) const char *name_space;
// 
// 	(+52) guint32    type_token;
// 	(+56) int        vtable_size; /* number of slots */
// 
// 	(+60) guint16     interface_count;
// 	(+64) guint32     interface_id;        /* unique inderface id (for interfaces) */
// 	(+68) guint32     max_interface_id;
// 	
// 	(+72) guint16     interface_offsets_count;
// 	(+76) MonoClass **interfaces_packed;
// 	(+80) guint16    *interface_offsets_packed;
// /* enabled only with small config for now: we might want to do it unconditionally */
// #ifdef MONO_SMALL_CONFIG
// #define COMPRESSED_INTERFACE_BITMAP 1
// #endif
// 	(+82) guint8     *interface_bitmap;
// 
// 	(+84) MonoClass **interfaces;
// 
// 	(+88) union {
// 		int class_size; /* size of area for static fields */
// 		int element_size; /* for array types */
// 		int generic_param_token; /* for generic param types, both var and mvar */
// 	} sizes;
// 
// 	/*
// 	 * Field information: Type and location from object base
// 	 */
// 	(+96) MonoClassField *fields; -> I confirmed this number in the disassembler too hmmmmm
// 
// 	(+96) MonoMethod **methods;
// 
// 	/* used as the type of the this argument and when passing the arg by value */
// 	(+100) MonoType this_arg;
// 	(+112) MonoType byval_arg;
// 
// 	(+124) MonoGCDescriptor gc_descr;
// 
// 	(+128) MonoClassRuntimeInfo *runtime_info;
// 
// 	/* Generic vtable. Initialized by a call to mono_class_setup_vtable () */
// 	(+132) MonoMethod **vtable;
// 
// 	/* Infrequently used items. See class-accessors.c: InfrequentDataKind for what goes into here. */
// 	(+136) MonoPropertyBag infrequent_data;
// 
// 	(+140) void *unity_user_data;
// };
class MonoClassMapper {
public:
    MonoClassMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr);

    // NO DATA IS LOADED BEFORE loadInner is called. This *can not* be a part of the 
    // constructor or else we'll risk having infinite recursion as it's possible for a
    // field to have a type to another class. If that type loads the class from the
    // image mapper and it's *this class* then the image mapper will try to load *this class*
    // again. Thus, the image mapper must first construct the wrapper, store the wrapper in
    // an accessible way, *and then* allow the class wrapper to load data from memory.
    void loadInner();

    const std::string& name() const { return _name; }
    std::string fullName() const;

    friend std::ostream& operator<<(std::ostream& os, const MonoClassMapper& map);
private:
    class MonoImageMapper* _image;
    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr = 0;

    std::string _name;
    std::string _namespace;
    std::unordered_map<std::string, MonoClassFieldMapperPtr> _fields;
};

std::ostream& operator<<(std::ostream& os, const MonoClassMapper& map);
using MonoClassMapperPtr = std::unique_ptr<MonoClassMapper>;

}