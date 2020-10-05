#pragma once

#include "shared/time.h"
#include "shared/valorant/valorant.h"
#include "valorant/valorant_match_details.h"

#include <memory>
#include <vector>
namespace service::valorant {

class ValorantApi;

// Stores information about a Valorant game.
// Can statefully add information about a Valorant game as well as back-fill information
// about the Valorant match via the API.
class ValorantMatch {
public:
    ValorantMatch(
        const shared::TimePoint& startTime,
        shared::valorant::EValorantMap map,
        const std::string& matchId
    );

    explicit ValorantMatch(ValorantMatchDetails&& details);

    const shared::TimePoint& startTime() const { return _startTime; }
    shared::valorant::EValorantMap map() const { return _map; }
    const std::string& matchId() const { return _matchId; }
    const shared::TimePoint& endTime() const { return _endTime; }
    bool isFinished() const { return _endTime != (shared::TimePoint::max)(); }
    
    const ValorantMatchDetails& details() const { return _details; }
    void setDetails(ValorantMatchDetails&& details) { _details = std::move(details); }

    void goToRoundState(const shared::TimePoint& tm, shared::valorant::EValorantRoundState state);
    void finishMatch(const shared::TimePoint& tm);

    // Returns whether or not this was successful.
    bool populateMatchDetailsFromApi(const ValorantApi* api);
private:
    shared::TimePoint _startTime;
    shared::valorant::EValorantMap _map;
    std::string _matchId;

    ValorantMatchDetails _details;
    shared::TimePoint _endTime;
};

using ValorantMatchPtr = std::unique_ptr<ValorantMatch>;

}