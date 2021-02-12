#pragma once

#include "process_watcher/memory/mono/mono_forward.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_box_visual_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using DeckPickerTrayDisplayMapperSPtr = std::shared_ptr<class DeckPickerTrayDisplayMapper>;
class DeckPickerTrayDisplayMapper {
public:
    static DeckPickerTrayDisplayMapperSPtr singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId);

    explicit DeckPickerTrayDisplayMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    CollectionDeckBoxVisualMapperSPtr getSelectedCustomDeckBox() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}