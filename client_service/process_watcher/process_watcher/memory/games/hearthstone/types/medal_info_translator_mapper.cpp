#include "process_watcher/memory/games/hearthstone/types/medal_info_translator_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string CURRENT_MEDAL_FIELD_NAME = "m_currMedalInfo";
const std::string CURRENT_WILD_MEDAL_FIELD_NAME = "m_currWildMedalInfo";

}

MedalInfoTranslatorMapper::MedalInfoTranslatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

TranslatedMedalInfoMapperSPtr MedalInfoTranslatorMapper::currentMedalInfo() const {
    const auto value = _object->get(CURRENT_MEDAL_FIELD_NAME);
    return std::make_shared<TranslatedMedalInfoMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

TranslatedMedalInfoMapperSPtr MedalInfoTranslatorMapper::currentWildMedalInfo() const {
    const auto value = _object->get(CURRENT_WILD_MEDAL_FIELD_NAME);
    return std::make_shared<TranslatedMedalInfoMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

std::ostream& operator<<(std::ostream& os, const MedalInfoTranslatorMapper& map) {
    os << "{Standard: " << *map.currentMedalInfo() << "\t" << "Wild: " << *map.currentWildMedalInfo() << "}";
    return os;
}

}