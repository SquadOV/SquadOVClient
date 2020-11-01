#include "process_watcher/memory/mono_memory_mapper.h"

#include "shared/log/log.h"
#include "shared/errors/error.h"

namespace process_watcher::memory {
namespace {

const std::string MONO_GET_ROOT_DOMAIN("mono_get_root_domain");
const std::string MONO_MAIN_ASSEMBLY_NAME("Assembly-CSharp");
constexpr uint8_t X86_MOV_EAX_OP = 0xa1;
constexpr uint8_t X86_RET_OP = 0xc3;
constexpr uint32_t MONO_DOMAIN_ASSEMBLIES_OFFSET =
    24 + // MonoCoopMutex    lock;
    4 + // MonoMemPool        *mp;
    4 + // MonoCodeManager    *code_mp;
    4 + // MonoAppDomainSetup *setup;
    4 + // MonoAppDomain      *domain;
    4 + // MonoAppContext     *default_context;
    4 + // MonoException      *out_of_memory_ex;
    4 + // MonoException      *null_reference_ex;
    4 + // MonoException      *stack_overflow_ex;
    4 + // MonoObject         *typeof_void;
    4 + // MonoObject         *ephemeron_tombstone;
    4 + // MonoArray          *empty_types;
    4 + // MonoString         *empty_string;
    4 + // MonoGHashTable     *env;
    4 + // MonoGHashTable     *ldstr_table;
    4 + // MonoGHashTable     *type_hash;
    4 + // MonoConcGHashTable     *refobject_hash;
    4 + // MonoGHashTable    *type_init_exception_hash;
    4 + // MonoGHashTable     *delegate_hash_table;
    4 + // guint32            state;
    4 + // gint32             domain_id;
    4;  // gint32             shadow_serial;
constexpr uint32_t MONO_DOMAIN_ID_OFFSET = MONO_DOMAIN_ASSEMBLIES_OFFSET - 8;
constexpr uint32_t MONO_ASSEMBLY_NAME_OFFSET = 8;
constexpr uint32_t MONO_ASSEMBLY_IMAGE_PTR_OFFSET = MONO_ASSEMBLY_NAME_OFFSET +
    4 + // const char *name;
    4 + // const char *culture;
    4 + // const char *hash_value;
    4 + // const mono_byte* public_key;
    20 + // mono_byte public_key_token [MONO_PUBLIC_KEY_TOKEN_LENGTH]; --> 17 + Padding to become a multiple of 4
    4 + // uint32_t hash_alg;
    4 + // uint32_t hash_len;
    4 + // uint32_t flags;
    (2 * 5) + // uint16_t major, minor, build, revision, arch;
    2; // padding
}

MonoMemoryMapper::MonoMemoryMapper(const ModuleMemoryMapperSPtr& memory, const PEMapper& pe) {
    // For now we only support 32-bit architectures.
    if (pe.arch() != ExecutableArch::X86) {
        THROW_ERROR("Only 32-bit executables supported for Mono.");
    }

    const auto rva = pe.getExportRva(MONO_GET_ROOT_DOMAIN);

    uint8_t x86Func[6];
    std::memcpy(x86Func, memory->moduleBuffer().data() + rva, 6);
    if (x86Func[0] != X86_MOV_EAX_OP || x86Func[5] != X86_RET_OP) {
        THROW_ERROR("Failed to find mono_get_root_domain");
    }
 

    // x86Func now contains an instruction of the form:
    // mov eax, [ptr]
    // In other words, eax (the returned root domain pointer) is set to the contents of
    // the 4 bytes in memory pointed to by [ptr].
    uint32_t rootDomainLocPtr = 0;
    std::memcpy(&rootDomainLocPtr, &x86Func[1], 4);
    memory->readProcessMemory(&_rootDomainPtr, static_cast<uintptr_t>(rootDomainLocPtr));

    // The domain ID is later needed to grab the VTable for classes/objects so we can extract properties and the like.
    memory->readProcessMemory(&_domainId, static_cast<uintptr_t>(_rootDomainPtr + MONO_DOMAIN_ID_OFFSET));

    // Now that we have the location of the root domain we need to grab the available assemblies
    // and find the relevant Assembly-CSharp.
    uint32_t domainAssembliesListPtr = 0;
    const auto domainAssembliesVA = _rootDomainPtr + MONO_DOMAIN_ASSEMBLIES_OFFSET;
    memory->readProcessMemory(&domainAssembliesListPtr, static_cast<uintptr_t>(domainAssembliesVA));

    // domainAssembliesListPtr is now the pointer to the first element of a GSList linked list of the
    // form:
    //
    // struct GSList {
    //  gpointer data;
    //  GSList *next;
    // };
    //
    // Note that we're assuming a *32* bit executable right now so we just need to read 8 bytes each time
    // we read a GSList.
    std::vector<char> nameBuffer(1024);
    while (domainAssembliesListPtr) {
        uint32_t data = 0;
        uint32_t next = 0;
        memory->readProcessMemory(&data, static_cast<uintptr_t>(domainAssembliesListPtr));
        memory->readProcessMemory(&next, static_cast<uintptr_t>(domainAssembliesListPtr + 4));

        // 'data' now is a pointer to a MonoAssembly object.
        // Use MONO_ASSEMBLY_NAME_OFFSET to get the location of the MonoAssemblyName *object* whose first 4 bytes are 
        // a pointer to a string (const char*)
        uint32_t namePointer = 0;
        memory->readProcessMemory(&namePointer, static_cast<uintptr_t>(data + MONO_ASSEMBLY_NAME_OFFSET));
        memory->readProcessMemory(nameBuffer, static_cast<uintptr_t>(namePointer), nameBuffer.size());

        if (std::string(nameBuffer.data()) != MONO_MAIN_ASSEMBLY_NAME) {
            domainAssembliesListPtr = next;
            continue;
        }

        // The next 4 bytes in the MonoAssembly object is a pointer to a MonoImage. Since we've found the correct assembly,
        // we'll go ahead and store our own representation of the MonoImage for easy access.
        uint32_t imagePointer = 0;
        memory->readProcessMemory(&imagePointer, static_cast<uintptr_t>(data + MONO_ASSEMBLY_IMAGE_PTR_OFFSET));

        _image = std::make_unique<mono::MonoImageMapper>(memory, imagePointer);
        break;
    }

    if (!_image) {
        THROW_ERROR("Failed to find the Assembly-CSharp main assembly.");
    }
}

std::ostream& operator<<(std::ostream& os, const MonoMemoryMapper& map) {
    os << "Mono Mapper: ";
    if (map._image) {
        os << std::endl << *map._image;
    } else {
        os << " INVALID";
    } 
    return os;
}

}