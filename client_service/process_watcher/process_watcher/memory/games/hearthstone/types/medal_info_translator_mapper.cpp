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
    if (!_object) {
        return nullptr;
    }
    const auto value = _object->get(CURRENT_MEDAL_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<TranslatedMedalInfoMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

TranslatedMedalInfoMapperSPtr MedalInfoTranslatorMapper::currentWildMedalInfo() const {
    if (!_object) {
        return nullptr;
    }
    const auto value = _object->get(CURRENT_WILD_MEDAL_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<TranslatedMedalInfoMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

nlohmann::json MedalInfoTranslatorMapper::toJson() const {
    const auto standard = currentMedalInfo();
    const auto wild = currentWildMedalInfo();

    if (!standard) {
        LOG_WARNING("Failed to obtain Hearthstone standard medal info." << std::endl);
    }

    if (!wild) {
        LOG_WARNING("Failed to obtain Hearthstone wild medal info." << std::endl);
    }

    return {
        { "standard", !!standard ? standard->toJson() : TranslatedMedalInfoMapper::emptyJson() },
        { "wild", !!wild ? wild->toJson() : TranslatedMedalInfoMapper::emptyJson() }
    };
}

std::ostream& operator<<(std::ostream& os, const MedalInfoTranslatorMapper& map) {
    const auto standard = map.currentMedalInfo();
    const auto wild = map.currentWildMedalInfo();

    os << "{Standard: ";
    if (!!standard) {
        os << *standard;
    } else {
        os << "None";
    }

    os << "\t" << "Wild: ";
    if (!!wild) {
        os << *wild;
    } else {
        os << "None";
    }
    
    os << "}";
    return os;
}

}