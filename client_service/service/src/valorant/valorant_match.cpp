#include "valorant/valorant_match.h"

#include "shared/log/log.h"
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

void ValorantMatch::goToRoundState(const shared::TimePoint& tm, shared::valorant::EValorantRoundState state) {
    if (isFinished()) {
        return;
    }

    if (state == _lastState) {
        // Some rare situation where VALORANT didn't feel like printing out a statement to demark the start of a round (buy or play).
        // We can error correct if we're just missing the statement for one event (one buy or one play).
        if (state == shared::valorant::EValorantRoundState::Buy) {
            // We skipped the previous play around. We can take a gander at to what the actual time is though since
            // it's pretty consistently the buy round time + 30.3 seconds.
            _rounds.back().startTime = _rounds.back().buyTime + std::chrono::milliseconds(30300);
        } else if (state == shared::valorant::EValorantRoundState::Play) {
            // We skipped the previous buy round. We can backfill in the previous buy round by subtracting 30.3 seconds (exact opposite of skipping the buy round).
            ValorantRoundMetadata metadata;
            metadata.round = _rounds.size();
            metadata.buyTime = tm - std::chrono::milliseconds(30300);
            _rounds.push_back(metadata);    
        }
    }

    if (state == shared::valorant::EValorantRoundState::Buy) {
        ValorantRoundMetadata metadata;
        metadata.round = _rounds.size();
        metadata.buyTime = tm;
        _rounds.push_back(metadata);
    } else if (state == shared::valorant::EValorantRoundState::Play && _rounds.size() > 0) {
        _rounds.back().startTime = tm;
    }
    _lastState = state;
}

void ValorantMatch::finishMatch(const shared::TimePoint& tm) {
    _endTime = tm;
}

nlohmann::json ValorantRoundMetadata::toJson() const {
    nlohmann::json json = nlohmann::json::object();
    json["round"] = round;
    if (shared::isTimeValid(buyTime)) {
        json["buyTime"] = shared::timeToIso(buyTime);
    } else {
        json["buyTime"] = nlohmann::json();
    }

    if (shared::isTimeValid(startTime)) {
        json["roundTime"] = shared::timeToIso(startTime);
    } else {
        json["roundTime"] = nlohmann::json();
    }
    
    return json;
}

nlohmann::json ValorantMatch::toJson() const {
    nlohmann::json json = nlohmann::json::object();
    json["startTime"] = shared::timeToIso(_startTime);
    json["endTime"] = shared::timeToIso(_endTime);
    json["rounds"] = nlohmann::json::array();
    for (const auto& rnd : _rounds) {
        json["rounds"].push_back(rnd.toJson());
    }

    return json;
}

}