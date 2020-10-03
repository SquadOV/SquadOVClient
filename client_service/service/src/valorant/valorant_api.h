#pragma once

#include "valorant/valorant_match_details.h"
#include "shared/riot/riot.h"
#include "http/http_client.h"

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace service::valorant {

struct CommonApiOptions {
    std::string apiServer;
};

class ValorantApi {
public:
    explicit ValorantApi(const shared::riot::RiotRsoToken& token);
    ~ValorantApi();
    void initializePvpServer(const std::string& server);
    bool ready() const { return !!_pvpClient; }

    // Get match details - round info, kills. Who's on what team. The score. Damage done. Everything.
    ValorantMatchDetailsPtr getMatchDetails(const std::string& matchId) const;
    std::string getLatestMatchId(const std::string& puuid, size_t* numMatches = nullptr) const;
    std::vector<std::string> getMatchHistoryIds(const std::string& puuid) const;
    std::vector<ValorantMatchDetailsPtr> getFullMatchHistory(const std::string& puuid) const;

private:
    void refreshToken();

    bool _running = true;
    std::thread _refreshThread;
    shared::riot::RiotRsoToken _token;
    std::unique_ptr<http::HttpClient> _pvpClient;
};

using ValorantApiPtr = std::unique_ptr<ValorantApi>;

}