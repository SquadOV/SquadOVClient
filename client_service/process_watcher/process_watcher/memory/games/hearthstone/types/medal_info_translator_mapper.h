#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/translated_medal_info_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/collection_deck_mapper.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace process_watcher::memory::games::hearthstone::types {

using MedalInfoTranslatorMapperSPtr = std::shared_ptr<class MedalInfoTranslatorMapper>;
class MedalInfoTranslatorMapper {
public:
    explicit MedalInfoTranslatorMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    TranslatedMedalInfoMapperSPtr currentStandardMedalInfo() const;
    TranslatedMedalInfoMapperSPtr currentWildMedalInfo() const;

    nlohmann::json toJson() const;
private:
    std::unordered_map<int32_t, TranslatedMedalInfoMapperSPtr> currentMedalInfo() const;

    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

std::ostream& operator<<(std::ostream& os, const MedalInfoTranslatorMapper& map);

}