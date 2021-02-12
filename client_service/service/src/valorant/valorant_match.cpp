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

    if (state == shared::valorant::EValorantRoundState::Buy) {
        ValorantRoundMetadata metadata;
        metadata.round = _rounds.size();
        metadata.buyTime = tm;
        _rounds.push_back(metadata);
    } else if (state == shared::valorant::EValorantRoundState::Play && _rounds.size() > 0) {
        _rounds.back().startTime = tm;
    }
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