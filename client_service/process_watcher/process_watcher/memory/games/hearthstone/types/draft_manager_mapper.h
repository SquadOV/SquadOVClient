#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using DraftManagerMapperSPtr = std::shared_ptr<class DraftManagerMapper>;
class DraftManagerMapper {
public:
    explicit DraftManagerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static const process_watcher::memory::mono::MonoClassMapper* klass(const process_watcher::memory::mono::MonoImageMapper& image);

    CollectionDeckMapperSPtr getDraftDeck() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}