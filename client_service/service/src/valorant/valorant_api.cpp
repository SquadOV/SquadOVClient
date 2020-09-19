#include "valorant/valorant_api.h"

#include "shared/errors/error.h"

#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
using namespace std::literals::chrono_literals;

namespace service::valorant {

ValorantApi::ValorantApi(const std::string& rsoToken, const std::string& entitlementToken):
    _rsoToken(rsoToken),
    _entitlementToken(entitlementToken) {
}

void ValorantApi::initializePvpServer(const std::string& server) {
    std::ostringstream fullHost;
    fullHost << "https://" << server;

    _pvpClient = std::make_unique<http::HttpClient>(fullHost.str());
    _pvpClient->setBearerAuthToken(_rsoToken.c_str());
    _pvpClient->setHeaderKeyValue("X-Riot-Entitlements-JWT", _entitlementToken);
}

void ValorantApi::reinitTokens(const std::string& rsoToken, const std::string& entitlementToken) {
    if (!_pvpClient) {
        throw std::runtime_error("Can't reinit tokens when the client has not yet been initialized.");
    }

    _pvpClient->setBearerAuthToken(_rsoToken.c_str());
    _pvpClient->setHeaderKeyValue("X-Riot-Entitlements-JWT", _entitlementToken);
}

ValorantMatchDetailsPtr ValorantApi::getMatchDetails(const std::string& matchId) const {
    std::ostringstream path;
    path << "/match-details/v1/matches/" << matchId;

    const auto result = _pvpClient->Get(path.str().c_str());

    if (result->status != 200) {
        return nullptr;
    }

    ValorantMatchDetailsPtr match = std::make_unique<ValorantMatchDetails>();
    match->parseFromJson(result->body);
    return match;
}

std::vector<ValorantMatchDetailsPtr> ValorantApi::getFullMatchHistory(const std::string& puuid) const {
    constexpr int numPerRequest = 20;
    int startIndex = 0;
    std::vector<ValorantMatchDetailsPtr> ret;
    std::vector<std::string> matches;

    while (true) {
        std::ostringstream path;
        const auto endIndex = startIndex + numPerRequest;
        path << "/match-history/v1/history/" << puuid
            << "?startIndex=" << startIndex
            << "&endIndex=" << endIndex;
        
        const auto result = _pvpClient->Get(path.str().c_str());
        if (result->status != 200) {
            THROW_ERROR("\tFailed to get match history: " << result->status << "\t" << result->curlError);
            return ret;
        }

        const auto parsedJson = nlohmann::json::parse(result->body);
        const auto total = parsedJson["Total"].get<int>();

        for (const auto& match : parsedJson["History"]) {
            matches.push_back(match["MatchID"].get<std::string>());
        }

        if (endIndex >= total) {
            break;
        }

        startIndex = endIndex;

        // Put in sleeps to try to avoid running afoul of any sort of rate limiting that might exist.
        std::this_thread::sleep_for(500ms);
    }

    for (const auto& matchId : matches) {
        auto details = getMatchDetails(matchId);

        if (!!details) {
            ret.emplace_back(std::move(details));
        } else {
            THROW_ERROR("\tFailed to get match details: " << matchId);
        }

        // Pretty excessive sleep but this is being done in its own thread so probably OK.
        std::this_thread::sleep_for(1s);
    }

    return ret;
}

}