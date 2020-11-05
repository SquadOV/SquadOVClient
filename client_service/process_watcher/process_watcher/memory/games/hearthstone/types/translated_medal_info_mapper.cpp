#include "process_watcher/memory/games/hearthstone/types/translated_medal_info_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string LEAGUE_ID_FIELD_NAME = "leagueId";
const std::string EARNED_STARS_FIELD_NAME = "earnedStars";
const std::string STAR_LEVEL_FIELD_NAME = "starLevel";
const std::string BEST_STAR_LEVEL_FIELD_NAME = "bestStarLevel";
const std::string WIN_STREAK_FIELD_NAME = "winStreak";
const std::string LEGEND_INDEX_FIELD_NAME = "legendIndex";

}

TranslatedMedalInfoMapper::TranslatedMedalInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

int32_t TranslatedMedalInfoMapper::leagueId() const {
    const auto value = _object->get(LEAGUE_ID_FIELD_NAME);
    return value.get<int32_t>();
}

int32_t TranslatedMedalInfoMapper::earnedStars() const {
    const auto value = _object->get(EARNED_STARS_FIELD_NAME);
    return value.get<int32_t>();
}

int32_t TranslatedMedalInfoMapper::starLevel() const {
    const auto value = _object->get(STAR_LEVEL_FIELD_NAME);
    return value.get<int32_t>();
}

int32_t TranslatedMedalInfoMapper::bestStarLevel() const {
    const auto value = _object->get(BEST_STAR_LEVEL_FIELD_NAME);
    return value.get<int32_t>();
}

int32_t TranslatedMedalInfoMapper::winStreak() const {
    const auto value = _object->get(WIN_STREAK_FIELD_NAME);
    return value.get<int32_t>();
}

int32_t TranslatedMedalInfoMapper::legendIndex() const {
    const auto value = _object->get(LEGEND_INDEX_FIELD_NAME);
    return value.get<int32_t>();
}

std::ostream& operator<<(std::ostream& os, const TranslatedMedalInfoMapper& map) {
    os << "[League: " << map.leagueId() 
        << " Earned: " << map.earnedStars()
        << " Stars: " << map.starLevel()
        << " Best Stars: " << map.bestStarLevel()
        << " Streak: " << map.winStreak()
        << " Legend: " << map.legendIndex()
        << "]";
    return os;
}

}