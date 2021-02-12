#include "process_watcher/memory/games/hearthstone/types/draft_manager_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {
const std::string DRAFTMANAGER_FULL_NAME("DraftManager");
const std::string DRAFTDECK_FIELD_NAME("m_draftDeck");
}

DraftManagerMapper::DraftManagerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

const process_watcher::memory::mono::MonoClassMapper* DraftManagerMapper::klass(const process_watcher::memory::mono::MonoImageMapper& image) {
    return image.loadClassFromFullName(DRAFTMANAGER_FULL_NAME);
}

CollectionDeckMapperSPtr DraftManagerMapper::getDraftDeck() const {
    const auto field = _object->get(DRAFTDECK_FIELD_NAME);
    const auto object = field.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<CollectionDeckMapper>(object);
}

}