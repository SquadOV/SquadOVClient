#include "process_watcher/memory/games/hearthstone/types/hearthstone_services_mapper.h"

namespace mono = process_watcher::memory::mono;

namespace process_watcher::memory::games::hearthstone::types {
namespace {
const std::string HEARTHSTONE_SERVICES_MAPPER_FULL_NAME = "HearthstoneServices";
const std::string DYNAMIC_SERVICES_FIELD_NAME = "s_dynamicServices";
const std::string SERVICES_FIELD_NAME = "s_services";
}

GameMgrMapperSPtr HearthstoneServicesMapper::getGameMgr(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = GameMgrMapper::klass(image);
    return get<GameMgrMapper>(cls, image, domainId);
}

DynamicServiceLocatorMapperPtr HearthstoneServicesMapper::getDynamicServices(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(HEARTHSTONE_SERVICES_MAPPER_FULL_NAME);
    const auto& field = cls->field(DYNAMIC_SERVICES_FIELD_NAME);
    const auto value = field.getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<DynamicServiceLocatorMapper>(value.get<mono::MonoObjectMapperSPtr>());
}

ServiceLocatorMapperPtr HearthstoneServicesMapper::getServices(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(HEARTHSTONE_SERVICES_MAPPER_FULL_NAME);
    const auto& field = cls->field(SERVICES_FIELD_NAME);
    const auto value = field.getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<ServiceLocatorMapper>(value.get<mono::MonoObjectMapperSPtr>());
}

}