#include "process_watcher/memory/games/hearthstone/types/dynamic_service_locator_mapper.h"

namespace mono = process_watcher::memory::mono;
namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string SERVICE_LOCATOR_FIELD_NAME = "m_serviceLocator";

}

DynamicServiceLocatorMapper::DynamicServiceLocatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {    
}

ServiceLocatorMapperPtr DynamicServiceLocatorMapper::serviceLocator() const {
    const auto value = _object->get(SERVICE_LOCATOR_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<ServiceLocatorMapper>(value.get<mono::MonoObjectMapperSPtr>());
}

}