#pragma once

#include "process_watcher/memory/mono/mono_forward.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using CollectionDeckMapperSPtr = std::shared_ptr<class CollectionDeckMapper>;
class CollectionDeckMapper {
public:
    explicit CollectionDeckMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    std::string name() const;
    int64_t deckId() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}