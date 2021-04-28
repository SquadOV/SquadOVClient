#pragma once

#include <cstrike15_gcmessages.pb.h>
#include <vector>

namespace steamworks_interface::csgo {

class CsgoClient {
public:
    void performHandshake() const;
    protobuf::csgo::CMsgGCCStrike15_v2_MatchList getRecentCsgoMatchList(uint32_t accountId) const;
};

}