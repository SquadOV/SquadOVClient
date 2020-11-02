#include "process_watcher/memory/mono/mono_classfield_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_vtable_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/string_mapper.h"

namespace process_watcher::memory::mono {

MonoClassFieldMapper::MonoClassFieldMapper(class MonoImageMapper* image, const process_watcher::memory::ModuleMemoryMapperSPtr& memory, uintptr_t ptr):
    _image(image),
    _memory(memory),
    _ptr(ptr) {
    _memory->readProcessMemory(&_classfield, ptr);
    _memory->readProcessMemory(_name, _classfield.namePtr, false);
    _class = _image->loadClassFromPtr(static_cast<uintptr_t>(_classfield.parentPtr));
    _type = _image->loadTypeFromPtr(static_cast<uintptr_t>(_classfield.typePtr));
}

DynamicMonoType MonoClassFieldMapper::getStatic(int32_t domainId) const {
    return get(nullptr, domainId);
}

DynamicMonoType MonoClassFieldMapper::get(const MonoObjectMapper* object, int32_t domainId) const {
    uintptr_t dataPtr = 0;

    if (!object) {
        if (!_type->isStatic()) {
            THROW_ERROR("Requested field is not static.");
        }

        if (_classfield.offset == -1) {
            THROW_ERROR("Special statics not yet supported.");
        }

        // According to Mono's code comments, the static field is
        // 'offset' away "from the start of the memory chunk allocated
        // for statics for the class." Another comment mentions that
        // "Static fields are stored in MonoVTable->data." However, no
        // such field exists. However, if we look at the "mono_vtable_get_static_field_data"
        // function we can see that MonoVTable has a member element called vtable and its
        // last element (if it has a static fields) is a pointer to the
        // memory chunk where static data resides. So we get a pointer to the
        // static data in the vtable member and offset it by what's stored in
        // _classfield.offset and that should get us what we need to read.
        const auto* vtable = _image->loadVTableForClass(_class, domainId);
        if (!vtable) {
            return DynamicMonoType(nullptr);
        }

        auto staticPtr = vtable->getStaticDataPtr();
        if (!staticPtr) {
            return DynamicMonoType(nullptr);
        }
        dataPtr = staticPtr + _classfield.offset;
    } else {
        dataPtr = object->ptr() + _classfield.offset;
    }

    const MonoClassMapper* objectKlass = nullptr;
    // isPtr determines whether or not what's stored in memory that we're going to read 
    // using the field's offset is a pointer or not. Note that this kind of mixes together
    // references and pointers as we're not really interested in maintaining separation
    // between the two.
    bool isPtr = false;
    switch (_type->type()) {
        case MonoTypes::Class:
        case MonoTypes::SzArray:
        case MonoTypes::String:
            isPtr = true;
            break;
        case MonoTypes::GenericInst: {
            // GenericInst is a ref/ptr only if the inner type isn't a value type.
            const auto* innerCls = _type->inner<const class MonoClassMapper*>();
            isPtr = !innerCls->isValueType();
            if (!isPtr) {
                objectKlass = innerCls;
            }
            break;
        }
        default:
            isPtr = _type->byRef();
            break;
    };

    if (isPtr) {
        uint32_t newPtr = 0;
        _memory->readProcessMemory(&newPtr, dataPtr);
        if (!newPtr) {
            return DynamicMonoType(nullptr);
        }
        dataPtr = static_cast<uintptr_t>(newPtr);
    }

    // TODO: Support other data types.
    DynamicMonoType ret(dataPtr);
    switch (_type->type()) {
        case MonoTypes::Class:
        case MonoTypes::SzArray:
        case MonoTypes::GenericInst:
            // We use the constructor that takes in the class because that allows the MonoObjectMapper to
            // selectively load the vtable or not based on data from the class.
            ret = std::make_shared<MonoObjectMapper>(_image, _memory, dataPtr, domainId, objectKlass);
            break;
        case MonoTypes::Uint1:
        case MonoTypes::Bool:
            ret = _memory->readProcessMemory<uint8_t>(dataPtr);
            break;
        case MonoTypes::Uint2:
            ret = _memory->readProcessMemory<uint16_t>(dataPtr);
            break;
        case MonoTypes::Uint4:
            ret = _memory->readProcessMemory<uint32_t>(dataPtr);
            break;
        case MonoTypes::Uint8:
            ret = _memory->readProcessMemory<uint64_t>(dataPtr);
            break;
        case MonoTypes::Int1:
            ret = _memory->readProcessMemory<int8_t>(dataPtr);
            break;
        case MonoTypes::Int2:
            ret = _memory->readProcessMemory<int16_t>(dataPtr);
            break;
        case MonoTypes::Int4:
            ret = _memory->readProcessMemory<int32_t>(dataPtr);
            break;
        case MonoTypes::Int8:
            ret = _memory->readProcessMemory<int64_t>(dataPtr);
            break;
        case MonoTypes::String: {
            // Technically this *could* be the same as the MonoTypes::Class case as it's really just
            // returning a MonoObject with class String; however, it's more useful to us if we abstract
            // that away and just return an std::string instead.
            auto monoObject = std::make_shared<MonoObjectMapper>(_image, _memory, dataPtr, domainId);
            types::StringMapper strMapper(monoObject);
            ret = strMapper.value();
            break;
        }
        case MonoTypes::Char:
            ret = _memory->readProcessMemory<char16_t>(dataPtr);
            break;
        default:
            THROW_ERROR("Unsupported type to get value from.");
    }
    return ret;
}

std::ostream& operator<<(std::ostream& os, const MonoClassFieldMapper& map) {
    os << "Field: " << map._name;
    os << std::endl << "Type: " << *map._type;
    return os;
}

}