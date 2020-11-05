#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

using TranslatedMedalInfoMapperSPtr = std::shared_ptr<class TranslatedMedalInfoMapper>;
class TranslatedMedalInfoMapper {
public:
    explicit TranslatedMedalInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    int32_t leagueId() const;
    int32_t earnedStars() const;
    int32_t starLevel() const;
    int32_t bestStarLevel() const;
    int32_t winStreak() const;
    int32_t legendIndex() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

std::ostream& operator<<(std::ostream& os, const TranslatedMedalInfoMapper& map);

}