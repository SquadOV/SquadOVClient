#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/game_mgr_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/draft_manager_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/service_locator_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/dynamic_service_locator_mapper.h"
#include <string>

namespace process_watcher::memory::games::hearthstone::types {

class HearthstoneServicesMapper {
public:
    static GameMgrMapperSPtr getGameMgr(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);
    static DraftManagerMapperSPtr getDraftManager(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);
private:
    // T is the type that we want to return.
    // K is the MonoClass that it corresponds to.
    template<typename T>
    static std::shared_ptr<T> get(const process_watcher::memory::mono::MonoClassMapper* typ, const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
        // We need to either search s_dynamicServices or s_services for the
        // given service.
        auto dynLoc = getDynamicServices(image, domainId);
        if (dynLoc) {
            return dynLoc->get<T>(typ);
        }

        auto loc = getServices(image, domainId);
        if (loc) {
            return loc->get<T>(typ);
        }

        return nullptr;
    }

    static DynamicServiceLocatorMapperPtr getDynamicServices(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);
    static ServiceLocatorMapperPtr getServices(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);
};

}