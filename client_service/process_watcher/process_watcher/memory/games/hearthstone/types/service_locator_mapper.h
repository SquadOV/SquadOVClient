#pragma once

#include "process_watcher/memory/mono/mono_class_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/list_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using ServiceLocatorMapperPtr = std::shared_ptr<class ServiceLocatorMapper>;
class ServiceLocatorMapper {
public:
    explicit ServiceLocatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    template<typename T>
    std::shared_ptr<T> get(const process_watcher::memory::mono::MonoClassMapper* cmpTyp) const {
        const auto allServices = services();
        const auto arr = allServices.items();
        for (int32_t i = 0; i < allServices.size(); ++i) {
            // We need to see if this type is equal to the type given by the caller
            // which involves seeing if the System.RuntimeType is equivalent to a MonoType.
            // Tricky -- dealt with in the RuntimeTypeMapper class. :)
            if (arr[i].serviceTypeName() == cmpTyp->fullName()) {
                return std::make_shared<T>(arr[i].service());
            }
        }
        return nullptr;
    }
    
private:
    class ServiceInfoMapper {
    public:
        explicit ServiceInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

        process_watcher::memory::mono::MonoObjectMapperSPtr service() const;
        std::string serviceTypeName() const;
    private:
        process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    };

    process_watcher::memory::mono::types::ListMapper<ServiceInfoMapper> services() const;

    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}