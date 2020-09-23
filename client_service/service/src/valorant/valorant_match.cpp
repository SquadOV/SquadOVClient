#include "valorant/valorant_match.h"
#include "valorant/valorant_api.h"

#include <iostream>

namespace service::valorant {

ValorantMatch::ValorantMatch(
    const shared::TimePoint& startTime,
    shared::valorant::EValorantMap map,
    const std::string& matchId):
    _startTime(startTime),
    _map(map),
    _matchId(matchId),
    _endTime(shared::TimePoint::max()) {
}

ValorantMatch::ValorantMatch(ValorantMatchDetails&& details):
    _startTime(shared::TimePoint::max()),
    _map(shared::valorant::mapIdToValorantMap(details.map())),
    _matchId(details.matchId()),
    _details(std::move(details)),
    _endTime(shared::TimePoint::max()) {
}

void ValorantMatch::populateMatchDetailsFromApi(const ValorantApi* api) {
    auto apiDetails = api->getMatchDetails(_matchId);
    // Didn't work - oops. Merge this in later I guess.
    if (!apiDetails) {
        std::cerr << "Failed to populate match details from API: " << _matchId << std::endl;
        return;
    }
    _details.mergeWithApi(apiDetails.get());
}

void ValorantMatch::goToRoundState(const shared::TimePoint& tm, shared::valorant::EValorantRoundState state) {
    if (isFinished()) {
        return;
    }

    _details.nextRoundStateWithTimestamp(tm, state);
}

void ValorantMatch::finishMatch(const shared::TimePoint& tm) {
    _endTime = tm;
}

}