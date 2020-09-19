#pragma once

#include "valorant/valorant_match_details.h"
#include "http/http_client.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace service::valorant {

struct CommonApiOptions {
    std::string apiServer;
};

class ValorantApi {
public:
    ValorantApi(const std::string& rsoToken, const std::string& entitlementToken);
    void initializePvpServer(const std::string& server);
    void reinitTokens(const std::string& rsoToken, const std::string& entitlementToken);

    // Get match details - round info, kills. Who's on what team. The score. Damage done. Everything.
    ValorantMatchDetailsPtr getMatchDetails(const std::string& matchId) const;
    std::vector<ValorantMatchDetailsPtr> getFullMatchHistory(const std::string& puuid) const;

private:
    std::string _rsoToken;
    std::string _entitlementToken;
    std::unique_ptr<http::HttpClient> _pvpClient;
};

using ValorantApiPtr = std::unique_ptr<ValorantApi>;

}