#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/nullable_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using GameMgrMapperSPtr = std::shared_ptr<class GameMgrMapper>;
class GameMgrMapper {
public:
    explicit GameMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static const process_watcher::memory::mono::MonoClassMapper* klass(const process_watcher::memory::mono::MonoImageMapper& image);

    process_watcher::memory::mono::types::NullableMapper<int64_t> lastDeckId() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}