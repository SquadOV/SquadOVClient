#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/find_game_result_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

class NetworkMapper {
public:
    explicit NetworkMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static const process_watcher::memory::mono::MonoClassMapper* klass(const process_watcher::memory::mono::MonoImageMapper& image);

    class NetworkStateMapper {
    public:
        explicit NetworkStateMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

        FindGameResultMapperSPtr getLastFindGameParameters() const;
    private:
        process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    };
    using NetworkStateMapperSPtr = std::shared_ptr<NetworkStateMapper>;
    NetworkStateMapperSPtr getState() const;

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};
using NetworkMapperSPtr = std::shared_ptr<NetworkMapper>;

}