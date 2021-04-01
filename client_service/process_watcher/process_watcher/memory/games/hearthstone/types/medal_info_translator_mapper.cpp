#include "process_watcher/memory/games/hearthstone/types/medal_info_translator_mapper.h"
#include "process_watcher/memory/mono/types/map_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string CURRENT_MEDAL_FIELD_NAME = "m_currMedalInfo";

}

MedalInfoTranslatorMapper::MedalInfoTranslatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

TranslatedMedalInfoMapperSPtr MedalInfoTranslatorMapper::currentStandardMedalInfo() const {
    const auto mapping = currentMedalInfo();
    auto it = mapping.find(static_cast<int>(FormatType::Standard));
    if (it == mapping.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

TranslatedMedalInfoMapperSPtr MedalInfoTranslatorMapper::currentWildMedalInfo() const {
    const auto mapping = currentMedalInfo();
    auto it = mapping.find(static_cast<int>(FormatType::Wild));
    if (it == mapping.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

std::unordered_map<int32_t, TranslatedMedalInfoMapperSPtr> MedalInfoTranslatorMapper::currentMedalInfo() const {
    const auto value = _object->get(CURRENT_MEDAL_FIELD_NAME);
    if (value.isNull()) {
        return {};
    }

    const process_watcher::memory::mono::types::MapMapper<int32_t, TranslatedMedalInfoMapperSPtr> mapper(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
    return mapper.values();
}

nlohmann::json MedalInfoTranslatorMapper::toJson() const {
    const auto standard = currentStandardMedalInfo();
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
    const auto standard = map.currentStandardMedalInfo();
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