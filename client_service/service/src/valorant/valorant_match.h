#pragma once

#include "shared/time.h"
#include "shared/valorant/valorant.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <vector>
namespace service::valorant {

class ValorantApi;

struct ValorantRoundMetadata {
    size_t round = 0;
    shared::TimePoint buyTime = shared::TimePoint::max();
    shared::TimePoint startTime = shared::TimePoint::max();

    nlohmann::json toJson() const;
};

// Stores information about a Valorant game from the *player's* perspective.
class ValorantMatch {
public:
    ValorantMatch(
        const shared::TimePoint& startTime,
        shared::valorant::EValorantMap map,
        const std::string& matchId
    );

    const shared::TimePoint& startTime() const { return _startTime; }
    shared::valorant::EValorantMap map() const { return _map; }
    const std::string& matchId() const { return _matchId; }
    const shared::TimePoint& endTime() const { return _endTime; }
    bool isFinished() const { return _endTime != (shared::TimePoint::max)(); }
    
    void goToRoundState(const shared::TimePoint& tm, shared::valorant::EValorantRoundState state);
    void finishMatch(const shared::TimePoint& tm);

    nlohmann::json toJson() const;
private:
    shared::TimePoint _startTime;
    shared::TimePoint _endTime;
    shared::valorant::EValorantMap _map;
    std::vector<ValorantRoundMetadata> _rounds;
    std::string _matchId;
    shared::valorant::EValorantRoundState _lastState = shared::valorant::EValorantRoundState::Buy;
};

using ValorantMatchPtr = std::unique_ptr<ValorantMatch>;

}