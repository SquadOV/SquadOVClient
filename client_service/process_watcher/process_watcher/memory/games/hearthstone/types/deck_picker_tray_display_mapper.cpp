#include "process_watcher/memory/games/hearthstone/types/deck_picker_tray_display_mapper.h"
#include "process_watcher/memory/mono/mono_image_mapper.h"
#include <string>

using namespace process_watcher::memory;

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string DECK_PICKER_TRAY_DISPLAY_FULL_NAME("DeckPickerTrayDisplay");
const std::string DECK_PICKER_TRAY_DISPLAY_SINGLETON_FIELD("s_instance");
const std::string DECK_PICKER_TRAY_DISPLAY_SELECTEDCUSTOMDECKBOX_FIELD("m_selectedCustomDeckBox");

}

DeckPickerTrayDisplayMapperSPtr DeckPickerTrayDisplayMapper::singleton(const mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(DECK_PICKER_TRAY_DISPLAY_FULL_NAME);
    const auto& field = cls->field(DECK_PICKER_TRAY_DISPLAY_SINGLETON_FIELD);
    const auto value = field.getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }

    const auto object = value.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<DeckPickerTrayDisplayMapper>(object);
}

DeckPickerTrayDisplayMapper::DeckPickerTrayDisplayMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
}

CollectionDeckBoxVisualMapperSPtr DeckPickerTrayDisplayMapper::getSelectedCustomDeckBox() const {
    const auto selectedCustomDeckBox = _object->get(DECK_PICKER_TRAY_DISPLAY_SELECTEDCUSTOMDECKBOX_FIELD);
    if (selectedCustomDeckBox.isNull()) {
        return nullptr;
    }

    const auto object = selectedCustomDeckBox.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<CollectionDeckBoxVisualMapper>(object);
}

}