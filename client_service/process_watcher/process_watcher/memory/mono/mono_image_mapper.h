#pragma once

#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/mono/mono_class_mapper.h"
#include "process_watcher/memory/mono/mono_type_mapper.h"
#include "process_watcher/memory/mono/mono_vtable_mapper.h"

#include <memory>
#include <unordered_map>

namespace process_watcher::memory::mono {

// Pulls data out of memory that represents a MonoImage.
// struct _MonoImage {
// 	/*
// 	 * This count is incremented during these situations:
// 	 *   - An assembly references this MonoImage though its 'image' field
// 	 *   - This MonoImage is present in the 'files' field of an image
// 	 *   - This MonoImage is present in the 'modules' field of an image
// 	 *   - A thread is holding a temporary reference to this MonoImage between
// 	 *     calls to mono_image_open and mono_image_close ()
// 	 */
// (+0)	int   ref_count;
// 
// 	/* If the raw data was allocated from a source such as mmap, the allocator may store resource tracking information here. */
// (+4)	void *raw_data_handle;
// (+8)	char *raw_data;
// #ifdef IL2CPP_ON_MONO
// 	void* il2cpp_codegen_handle;
// #endif
// (+12)	guint32 raw_data_len;
// (+16)	guint8 raw_buffer_used    : 1;
//  guint8 raw_data_allocated : 1;
// 	guint8 fileio_used : 1;
// 
// #ifdef HOST_WIN32
// 	/* Module was loaded using LoadLibrary. */
// 	guint8 is_module_handle : 1;
// 
// 	/* Module entry point is _CorDllMain. */
// (+20)	guint8 has_entry_point : 1;
// #endif
// 
// 	/* Whenever this is a dynamically emitted module */
// 	guint8 dynamic : 1;
// 
// 	/* Whenever this is a reflection only image */
//	guint8 ref_only : 1;
// 
// 	/* Whenever this image contains uncompressed metadata */
// 	guint8 uncompressed_metadata : 1;
// 
// 	/* Whenever this image contains metadata only without PE data */
// 	guint8 metadata_only : 1;
// 
// 	/*  Whether this image belongs to load-from context */
// 	guint8 load_from_context: 1;
// 
// 	guint8 checked_module_cctor : 1;
// 	guint8 has_module_cctor : 1;
// 
// 	guint8 idx_string_wide : 1;
// 	guint8 idx_guid_wide : 1;
// 	guint8 idx_blob_wide : 1;
// 
// 	/* Whenever this image is considered as platform code for the CoreCLR security model */
// 	guint8 core_clr_platform_code : 1;
// 
// 	/* The path to the file for this image. */
// (+20)	char *name;
// 
// 	/* The assembly name reported in the file for this image (expected to be NULL for a netmodule) */
// (+24)	const char *assembly_name;
// 
// 	/* The module name reported in the file for this image (could be NULL for a malformed file) */
// (+28)	const char *module_name;
// 
// (+32)	char *version;
// (+36)	gint16 md_version_major, md_version_minor;
// (+40)	char *guid;
// (+44)	void *image_info;
// (+48)	MonoMemPool         *mempool; /*protected by the image lock*/
// 
// (+52)	char                *raw_metadata;
// 			    
// (+56)	MonoStreamHeader     heap_strings;
// (+64)	MonoStreamHeader     heap_us;
// (+72)	MonoStreamHeader     heap_blob;
// (+80)	MonoStreamHeader     heap_guid;
// (+88)	MonoStreamHeader     heap_tables;
// (+96)	MonoStreamHeader     heap_pdb;
// 			    
// (+104)	const char          *tables_base;
// 
// 	/* For PPDB files */
// (+108)	guint64 referenced_tables;
// (+116)	int *referenced_table_rows;
// 
// 	/**/
// (+120)	MonoTableInfo        tables [MONO_TABLE_NUM]; // MONO_TABLE_NUM = 56, sizeof(MonoTableInfo) = 12
// 
// 	/*
// 	 * references is initialized only by using the mono_assembly_open
// 	 * function, and not by using the lowlevel mono_image_open.
// 	 *
// 	 * It is NULL terminated.
// 	 */
// (+792)	MonoAssembly **references;
// (+796)	int nreferences;
// 
// 	/* Code files in the assembly. The main assembly has a "file" table and also a "module"
// 	 * table, where the module table is a subset of the file table. We track both lists,
// 	 * and because we can lazy-load them at different times we reference-increment both.
// 	 */
// (+800)	MonoImage **modules;
// (+804)	guint32 module_count;
// (+808)	gboolean *modules_loaded;
// 
// (+812)	MonoImage **files;
// (+816) guint32 file_count;
// 
// (+820)	gpointer aot_module;
// 
// (+824)	guint8 aotid[16];
// 
// 	/*
// 	 * The Assembly this image was loaded from.
// 	 */
// (+844)	MonoAssembly *assembly; -> I'm missing something here...I've verified +844 is the right offset but i can't seem to get the number to compute by hand. Version mismatch?? PADDING???
// 
// 	/*
// 	 * Indexed by method tokens and typedef tokens.
// 	 */
// (+848)	GHashTable *method_cache; /*protected by the image lock*/
// (+852)	MonoInternalHashTable class_cache;
// 
// 	/* Indexed by memberref + methodspec tokens */
// 	GHashTable *methodref_cache; /*protected by the image lock*/
// 
// 	/*
// 	 * Indexed by fielddef and memberref tokens
// 	 */
// 	MonoConcurrentHashTable *field_cache; /*protected by the image lock*/
// 
// 	/* indexed by typespec tokens. */
// 	MonoConcurrentHashTable *typespec_cache; /* protected by the image lock */
// 	/* indexed by token */
// 	GHashTable *memberref_signatures;
// 	GHashTable *helper_signatures;
// 
// 	/* Indexed by blob heap indexes */
// 	GHashTable *method_signatures;
// 
// 	/*
// 	 * Indexes namespaces to hash tables that map class name to typedef token.
// 	 */
// 	GHashTable *name_cache;  /*protected by the image lock*/
// 
// 	/*
// 	 * Indexed by MonoClass
// 	 */
// 	GHashTable *array_cache;
// 	GHashTable *ptr_cache;
// 
// 	GHashTable *szarray_cache;
// 	/* This has a separate lock to improve scalability */
// 	mono_mutex_t szarray_cache_lock;
// 
// 	/*
// 	 * indexed by SignaturePointerPair
// 	 */
// 	GHashTable *native_func_wrapper_cache;
// 
// 	/*
// 	 * indexed by MonoMethod pointers 
// 	 */
// 	GHashTable *runtime_invoke_vcall_cache;
// 	GHashTable *wrapper_param_names;
// 	GHashTable *array_accessor_cache;
// 
// 	/*
// 	 * indexed by MonoClass pointers
// 	 */
// 	GHashTable *ldfld_wrapper_cache;
// 	GHashTable *ldflda_wrapper_cache;
// 	GHashTable *stfld_wrapper_cache;
// 	GHashTable *isinst_cache;
// 
// 	GHashTable *icall_wrapper_cache;
// 	GHashTable *castclass_cache;
// 	GHashTable *proxy_isinst_cache;
// 	GHashTable *rgctx_template_hash; /* LOCKING: templates lock */
// 
// 	/* Contains rarely used fields of runtime structures belonging to this image */
// 	MonoPropertyHash *property_hash;
// 
// 	void *reflection_info;
// 
// 	/*
// 	 * user_info is a public field and is not touched by the
// 	 * metadata engine
// 	 */
// 	void *user_info;
// 
// 	/* dll map entries */
// 	MonoDllMap *dll_map;
// 
// 	/* interfaces IDs from this image */
// 	/* protected by the classes lock */
// 	MonoBitSet *interface_bitset;
// 
// 	/* when the image is being closed, this is abused as a list of
// 	   malloc'ed regions to be freed. */
// 	GSList *reflection_info_unregister_classes;
// 
// 	/* List of dependent image sets containing this image */
// 	/* Protected by image_sets_lock */
// 	GSList *image_sets;
// 
// 	/* Caches for wrappers that DO NOT reference generic */
// 	/* arguments */
// 	MonoWrapperCaches wrapper_caches;
// 
// 	/* Caches for MonoClass-es representing anon generic params */
// 	MonoClass **var_cache_fast;
// 	MonoClass **mvar_cache_fast;
// 	GHashTable *var_cache_slow;
// 	GHashTable *mvar_cache_slow;
// 	GHashTable *var_cache_constrained;
// 	GHashTable *mvar_cache_constrained;
// 
// 	/* Maps malloc-ed char* pinvoke scope -> MonoDl* */
// 	GHashTable *pinvoke_scopes;
// 
// 	/* Maps malloc-ed char* pinvoke scope -> malloced-ed char* filename */
// 	GHashTable *pinvoke_scope_filenames;
// 
// 	/* The loader used to load this image */
// 	MonoImageLoader *loader;
// 
// 	// Containers for MonoGenericParams associated with this image but not with any specific class or method. Created on demand.
// 	// This could happen, for example, for MonoTypes associated with TypeSpec table entries.
// 	MonoGenericContainer *anonymous_generic_class_container;
// 	MonoGenericContainer *anonymous_generic_method_container;
// 
// 	gboolean weak_fields_inited;
// 	/* Contains 1 based indexes */
// 	GHashTable *weak_field_indexes;
// 
// 	/*
// 	 * No other runtime locks must be taken while holding this lock.
// 	 * It's meant to be used only to mutate and query structures part of this image.
// 	 */
// 	mono_mutex_t    lock;
// };
class MonoImageMapper {
public:
    MonoImageMapper(const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr);

    const MonoClassMapper* loadClassFromPtr(uintptr_t ptr);
    const MonoClassMapper* loadClassFromFullName(const std::string& nm) const;

    const MonoTypeMapper* loadTypeFromPtr(uintptr_t ptr);

    // loadVTableForClass should be called on demand so that we only load the vtables for the
    // classes/objects that we need to actually access. Note that this is a bit of a hack to
    // get around the const-ness of the input klass. LOL.
    const MonoVTableMapper* loadVTableForClass(const MonoClassMapper* klass, int32_t domainId);
    const MonoVTableMapper* loadVTableFromPtr(uintptr_t ptr, int32_t domainId);

    friend std::ostream& operator<<(std::ostream& os, const MonoImageMapper& map);
private:
    process_watcher::memory::ModuleMemoryMapperSPtr _memory;
    uintptr_t _ptr = 0;

    // The pointer to the MonoClassMapper *must* be owned by _classPointers as
    // _classPointers is the only one guaranteed to have a unique mapping to classes.
    // Accessing class by names is *unsafe* for things like generic types but still needs
    // to be supported for easier user access.
    // Access by class name.
    std::unordered_map<std::string, MonoClassMapper*> _classes;
    // Access by class pointer.
    std::unordered_map<uintptr_t, MonoClassMapperPtr> _classPointers;

    // Access by type pointer.
    std::unordered_map<uintptr_t, MonoTypeMapperPtr> _types;

    // Populated by loadVTableForClass.
    std::unordered_map<uintptr_t, const MonoVTableMapper*> _vtables;
};

std::ostream& operator<<(std::ostream& os, const MonoImageMapper& map);

using MonoImageMapperPtr = std::unique_ptr<MonoImageMapper>;

}