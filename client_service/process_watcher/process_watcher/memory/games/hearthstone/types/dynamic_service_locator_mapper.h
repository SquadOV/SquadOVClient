#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/service_locator_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using DynamicServiceLocatorMapperPtr = std::shared_ptr<class DynamicServiceLocatorMapper>;
class DynamicServiceLocatorMapper {
public:
    explicit DynamicServiceLocatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    template<typename T>
    std::shared_ptr<T> get(const process_watcher::memory::mono::MonoClassMapper* typ) const {
        auto loc = serviceLocator();
        if (!loc) {
            return nullptr;
        }
        return loc->get<T>(typ);
    }

    ServiceLocatorMapperPtr serviceLocator() const;

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}