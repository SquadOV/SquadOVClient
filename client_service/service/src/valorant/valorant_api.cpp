#include "valorant/valorant_api.h"

#include "shared/errors/error.h"
#include "api/squadov_api.h"

#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
using namespace std::literals::chrono_literals;

namespace service::valorant {

ValorantApi::ValorantApi(const shared::riot::RiotRsoToken& token):
    _token(token) {
    refreshToken();
}

ValorantApi::~ValorantApi() {
    _running = false;
    _refreshThread.join();
}

void ValorantApi::initializePvpServer(const std::string& server) {
    std::ostringstream fullHost;
    fullHost << "https://" << server;

    _pvpClient = std::make_unique<http::HttpClient>(fullHost.str());
    _pvpClient->setBearerAuthToken(_token.rsoToken.c_str());
    _pvpClient->setHeaderKeyValue("X-Riot-Entitlements-JWT", _token.entitlementsToken);

    // 120 seconds per 100 tasks
    _pvpClient->setRateLimit(1.2);
}

void ValorantApi::refreshToken() {
    // Create a thread that'll sleep until the token is about to expire, then try to 
    // refresh it.
    _refreshThread = std::thread([this](){
        while (_running) {
            // First check if the token is expired. We'll say the token is expired if we're within 
            // 1 minute of being expired. Should account for any potential drift and allow us to
            // always maintain a valid token.
            const bool isExpired = (_token.expires - shared::nowUtc()) < std::chrono::minutes(1);

            // If not, sleep. However don't sleep until the token is expired as that might be
            // an hour away and the program might want to quit out of this loop earlier than that.
            if (!isExpired) {
                std::this_thread::sleep_for(1s);
                continue;
            }

            // If the token is expired by our metric then refresh the token using our API.
            _token = service::api::getGlobalApi()->refreshValorantRsoToken(_token.puuid);
            if (!!_pvpClient) {
                _pvpClient->setBearerAuthToken(_token.rsoToken.c_str());
                _pvpClient->setHeaderKeyValue("X-Riot-Entitlements-JWT", _token.entitlementsToken);
            }

            // Sleep for some time afterwards as we know the token is OK now.
            std::this_thread::sleep_for(1s);
        }
    });
}

nlohmann::json ValorantApi::getRawMatchDetails(const std::string& matchId) const {
    std::ostringstream path;
    path << "/match-details/v1/matches/" << matchId;

    const auto result = _pvpClient->get(path.str().c_str());

    if (result->status != 200) {
        THROW_ERROR("Failed to get Valorant match details: " << matchId << std::endl);
        return nlohmann::json();
    }

    return nlohmann::json::parse(result->body);
}

std::vector<std::string> ValorantApi::getMatchHistoryIds(const std::string& puuid) const {
    constexpr int numPerRequest = 20;
    int startIndex = 0;
    std::vector<std::string> matches;

    while (true) {
        std::ostringstream path;
        const auto endIndex = startIndex + numPerRequest;
        path << "/match-history/v1/history/" << puuid
            << "?startIndex=" << startIndex
            << "&endIndex=" << endIndex;
        
        const auto result = _pvpClient->get(path.str().c_str());
        if (result->status != 200) {
            THROW_ERROR("\tFailed to get match history: " << result->status << "\t" << result->curlError);
            return matches;
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
    }

    return matches;
}

}