#pragma once

#include "http/http_client.h"
#include "shared/riot/riot.h"
#include "shared/squadov/session.h"
#include "shared/squadov/vod.h"
#include "shared/aimlab/aimlab.h"

#include <functional>
#include <memory>

namespace service::api {

using SessionIdUpdateCallback = std::function<void(const std::string&)>;

// A mix between our local API (for however much longer we need to keep it around for)
// and our we bapi.
class SquadovApi {
public:
    SquadovApi();

    void setSessionIdUpdateCallback(const SessionIdUpdateCallback& cb) { _sessionUpdateCallback = cb; }
    void setSessionId(const std::string& key);

    const auto& getSession() const { return _session; }

    // User
    shared::squadov::SquadOVUser getCurrentUser() const;

    // Valorant
    shared::riot::RiotRsoToken refreshValorantRsoToken(const std::string& puuid) const;
    shared::riot::RiotUser getRiotUserFromPuuid(const std::string& puuid) const;

    std::string uploadValorantMatch(const std::string& matchId, const std::string& rawData) const;

    // Aim Lab
    std::string uploadAimlabTask(const shared::aimlab::TaskData& data) const;

    // VOD
    void associateVod(const shared::squadov::VodAssociation& association) const;
    void deleteVod(const std::string& vodUuid) const;

private:
    SessionIdUpdateCallback _sessionUpdateCallback;

    std::unique_ptr<http::HttpClient> _localClient;
    std::unique_ptr<http::HttpClient> _webClient;

    // I'm not a big fan of leaving this here...
    shared::squadov::SquadOVSessionStorage _session;
};

using SquadovApiPtr = std::unique_ptr<SquadovApi>;

SquadovApi* getGlobalApi();

}