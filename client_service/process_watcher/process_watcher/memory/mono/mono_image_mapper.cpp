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

MonoImageWrapper::MonoImageWrapper(const process_watcher::memory::ModuleMemoryMapper& memory, uintptr_t ptr):
    _ptr(ptr) {

    // Read the class_cache which has type MonoInternalHashTable.
    // We just have to read every entry in the hash table to obtain every class in this MonoImage.
    MonoInternalHashTable hashTable;
    memory.readProcessMemory(&hashTable, static_cast<uintptr_t>(_ptr + MONO_IMAGE_CLASS_CACHE_OFFSET));

    // The "tablePtr" is just an array of pointers of size "size". Out of the "size" entries, there should
    // be "numEntries" that are null. Each pointer points to an object of type MonoClass.
    for (int32_t i = 0; i < hashTable.size; ++i) {
        uint32_t hashtableEntryPtr = 0;
        memory.readProcessMemory(&hashtableEntryPtr, hashTable.tablePtr + sizeof(uint32_t) * i);

        if (!hashtableEntryPtr) {
            continue;
        }

        auto klass = std::make_unique<MonoClassMapper>(memory, hashtableEntryPtr);
        std::cout << "KLASS: " << klass->name() << "\t" << klass->fullName() << std::endl;
        _classes[klass->name()] = std::move(klass);
    }
}

}