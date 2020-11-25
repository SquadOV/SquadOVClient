#pragma once

#include "process_watcher/memory/mono/mono_forward.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/map_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using CollectionManagerMapperSPtr = std::shared_ptr<class CollectionManagerMapper>;
class CollectionManagerMapper {
public:
    static CollectionManagerMapperSPtr singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);

    explicit CollectionManagerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    CollectionDeckMapperSPtr getDeckFromId(int64_t deckId) const;
    CollectionDeckMapperSPtr getCurrentDuelsDeck() const;
private:
    using DeckMap = process_watcher::memory::mono::types::MapMapper<int64_t, process_watcher::memory::mono::MonoObjectMapperSPtr>;

    CollectionDeckMapperSPtr getDeckFromIdFromDeckMap(const std::string& fieldName, int64_t deckId) const;
    DeckMap getDeckMap(const std::string& fieldName) const;

    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}