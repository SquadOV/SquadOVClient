#include "steamworks_interface/csgo/csgo_client.h"
#include "steamworks_interface/client.h"
#include "shared/log/log.h"

#include <future>
#include <gcsystemmsgs.pb.h>
#include <gcsdk_gcmessages.pb.h>

namespace steamworks_interface::csgo {

void CsgoClient::performHandshake() const {
    auto* singleton = steamworks_interface::SteamworksClient::singleton();

    protobuf::csgo::CMsgClientHello hello;
    hello.set_client_session_need(1);

    LOG_INFO("Sending CSGO handshake..." << std::endl);
    singleton->sendGcMessageWithResponse(protobuf::csgo::k_EMsgGCClientHello, &hello, protobuf::csgo::k_EMsgGCClientWelcome, nullptr);
    LOG_INFO("...Successful CSGO handshake." << std::endl);
}

protobuf::csgo::CMsgGCCStrike15_v2_MatchList CsgoClient::getRecentCsgoMatchList(uint32_t accountId) const {
    auto* singleton = steamworks_interface::SteamworksClient::singleton();

    protobuf::csgo::CMsgGCCStrike15_v2_MatchListRequestRecentUserGames request;
    request.set_accountid(accountId);

    LOG_INFO("Sending CSGO match request..." << std::endl);

    std::optional<protobuf::csgo::CMsgGCCStrike15_v2_MatchList> matchlist;
    auto innerDelegate = std::make_shared<TypedSteamworksGcDelegate<protobuf::csgo::CMsgGCCStrike15_v2_MatchList>>([&matchlist](protobuf::csgo::CMsgGCCStrike15_v2_MatchList* data){
        matchlist = *data;
    });

    singleton->sendGcMessageWithResponse(protobuf::csgo::k_EMsgGCCStrike15_v2_MatchListRequestRecentUserGames, &request, protobuf::csgo::k_EMsgGCCStrike15_v2_MatchList, innerDelegate);

    if (!matchlist) {
        THROW_ERROR("Failed to obtain CSGO match list.");
    }
    return matchlist.value();
}

}