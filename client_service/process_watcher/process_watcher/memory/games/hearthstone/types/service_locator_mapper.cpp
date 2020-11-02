#include "process_watcher/memory/games/hearthstone/types/service_locator_mapper.h"
#include <string>

namespace mono = process_watcher::memory::mono;
namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string SERVICEINFO_SERVICE_FIELD = "<Service>k__BackingField";
const std::string SERVICEINFO_SERVICE_TYPE_NAME_FIELD = "<ServiceTypeName>k__BackingField";

const std::string SERVICE_SERVICES_FIELD = "m_services";

}

ServiceLocatorMapper::ServiceLocatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

mono::types::ListMapper<ServiceLocatorMapper::ServiceInfoMapper> ServiceLocatorMapper::services() const {
    const auto value = _object->get(SERVICE_SERVICES_FIELD);
    return mono::types::ListMapper<ServiceLocatorMapper::ServiceInfoMapper>(value.get<mono::MonoObjectMapperSPtr>());
}

ServiceLocatorMapper::ServiceInfoMapper::ServiceInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

process_watcher::memory::mono::MonoObjectMapperSPtr ServiceLocatorMapper::ServiceInfoMapper::service() const {
    return _object->get(SERVICEINFO_SERVICE_FIELD).get<mono::MonoObjectMapperSPtr>();
}

std::string ServiceLocatorMapper::ServiceInfoMapper::serviceTypeName() const {
    return _object->get(SERVICEINFO_SERVICE_TYPE_NAME_FIELD).get<std::string>();
}

}