#include "process_watcher/memory/mono/mono_image_mapper.h"

namespace process_watcher::memory::mono {
namespace {

constexpr uint32_t MONO_IMAGE_CLASS_CACHE_OFFSET = 852;

struct MonoInternalHashTable {
    uint32_t hashFunc;
    uint32_t keyExtract;
    uint32_t nextValue;
    int32_t size;
    int32_t numEntries;
    uint32_t tablePtr;
};

}

MonoImageMapper::MonoImageMapper(const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr):
    _memory(memory),
    _ptr(ptr) {

    // Read the class_cache which has type MonoInternalHashTable.
    // We just have to read every entry in the hash table to obtain every class in this MonoImage.
    MonoInternalHashTable hashTable;
    _memory->readProcessMemory(&hashTable, static_cast<uintptr_t>(_ptr + MONO_IMAGE_CLASS_CACHE_OFFSET));

    // The "tablePtr" is just an array of pointers of size "size". Out of the "size" entries, there should
    // be "numEntries" that are null. Each pointer points to an object of type MonoClass.
    for (int32_t i = 0; i < hashTable.size; ++i) {
        uint32_t hashtableEntryPtr = 0;
        _memory->readProcessMemory(&hashtableEntryPtr, hashTable.tablePtr + sizeof(uint32_t) * i);

        if (!hashtableEntryPtr) {
            continue;
        }

        loadClassFromPtr(static_cast<uintptr_t>(hashtableEntryPtr));
    }
}

const MonoClassMapper* MonoImageMapper::loadClassFromPtr(uintptr_t ptr) {
    auto it = _classPointers.find(ptr);
    if (it != _classPointers.end()) {
        return (*it).second;
    }

    auto klass = std::make_unique<MonoClassMapper>(this, _memory, ptr);
    auto* retPtr = klass.get();
    _classPointers[ptr] = retPtr;

    // This needs to be before we try to store the class by name.
    // It has to be after the ptr set though since Mono internals should
    // only try to look up the class using the ptr.
    retPtr->loadInner();

    _classes[klass->fullName()] = std::move(klass);
    return retPtr;
}

const MonoClassMapper* MonoImageMapper::loadClassFromFullName(const std::string& nm) const {
    return _classes.at(nm).get();
}

const MonoTypeMapper* MonoImageMapper::loadTypeFromPtr(uintptr_t ptr) {
    auto it = _types.find(ptr);
    if (it != _types.end()) {
        return (*it).second.get();
    }

    auto typ = std::make_unique<MonoTypeMapper>(this, _memory, ptr);
    auto* typPtr = typ.get();
    _types[ptr] = std::move(typ);
    typPtr->loadInner();
    return typPtr;
}

const MonoVTableMapper* MonoImageMapper::loadVTableForClass(const MonoClassMapper* klass, int32_t domainId) {
    // As I said, hack for getting around the const-ness...
    auto* cls = const_cast<MonoClassMapper*>(loadClassFromPtr(klass->ptr()));
    const auto* vtable = cls->loadVTable(domainId);
    _vtables[vtable->ptr()] = vtable;
    return vtable;
}

const MonoVTableMapper* MonoImageMapper::loadVTableFromPtr(uintptr_t ptr, int32_t domainId) {
    // Note that it's inappropriate to assume that loadVTableFromPtr is only called for
    // vtables that already exist. If it doesn't exist, we need to do some digging to figure out
    // what class it's being loaded for.
    auto it = _vtables.find(ptr);
    if (it != _vtables.end()) {
        return (*it).second;
    }

    // The first 4 bytes are a pointer to the class so just use that to go through the loadVTableForClass
    // route since we kind of assume that the class takes ownership of the vtable for better or worse.
    uint32_t classPtr = 0;
    _memory->readProcessMemory(&classPtr, ptr);
    const auto* cls = loadClassFromPtr(static_cast<uintptr_t>(classPtr));
    return loadVTableForClass(cls, domainId);
}

std::ostream& operator<<(std::ostream& os, const MonoImageMapper& map) {
    os << "MONO IMAGE";
    for (const auto& [name, cls] : map._classes) {
        os << std::endl << "##### " << name << " #####";
        os << std::endl << *cls;
    }
    return os;
}

}