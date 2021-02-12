#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/uber_text_mapper.h"
#include <memory>
#include <stdint.h>
#include <string>

namespace process_watcher::memory::games::hearthstone::types {

using CollectionDeckBoxVisualMapperSPtr = std::shared_ptr<class CollectionDeckBoxVisualMapper>;
class CollectionDeckBoxVisualMapper {
public:
    explicit CollectionDeckBoxVisualMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    UberTextSPtr deckName() const;
    int64_t deckId() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}