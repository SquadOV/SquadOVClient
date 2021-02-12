#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

class GameSessionInfoMapper {
public:
    explicit GameSessionInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    bool hasClientDeckId() const;
    int64_t clientDeckId() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};
using GameSessionInfoMapperSPtr = std::shared_ptr<GameSessionInfoMapper>;

}