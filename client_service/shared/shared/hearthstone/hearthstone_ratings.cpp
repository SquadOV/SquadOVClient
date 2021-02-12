#include "shared/hearthstone/hearthstone_ratings.h"

namespace shared::hearthstone {

nlohmann::json HearthstoneRatings::toJson() const {
    nlohmann::json body;
    if (battlegroundsRating) {
        body["battlegroundsRating"] = battlegroundsRating.value();
    } else {
        body["battlegroundsRating"] = nlohmann::json();
    }

    if (duelsCasualRating) {
        body["duelsCasualRating"] = duelsCasualRating.value();
    } else {
        body["duelsCasualRating"] = nlohmann::json();
    }

    if (duelsHeroicRating) {
        body["duelsHeroicRating"] = duelsHeroicRating.value();
    } else {
        body["duelsHeroicRating"] = nlohmann::json();
    }

    return body;
}

}