#include "game_event_watcher/csgo/csgo_gsi_packet.h"
#include "shared/io.h"

namespace game_event_watcher {

shared::TimePoint CsgoGsiPacket::timestamp() const {
    if (provider && provider->timestamp) {
        return shared::unixMsToTime(provider->timestamp.value() * 1000);
    } else {
        return shared::nowUtc();
    }
}

}


using namespace game_event_watcher;

std::ostream& operator<<(std::ostream& os, const CsgoGsiPacket& p) {
    os << "CsgoGsiPacket {" << std::endl
        << "\tmap: " << p.map << std::endl
        << "\tplayer: " << p.player << std::endl
        << "\tprovider: " << p.provider << std::endl
        << "\tround: " << p.round << std::endl;

    if (p.previous) {
        os << "\tprevious: " << *p.previous << std::endl;
    }

    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiTeamPacket& p) {
    os << "CsgoGsiTeamPacket {" << std::endl
        << "\tscore: " << p.score << std::endl
        << "\ttimeouts remaining: " << p.timeoutsRemaining << std::endl
        << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiMapPacket& p) {
    os << "CsgoGsiMapPacket {" << std::endl
        << "\tspectators: " << p.currentSpectators << std::endl
        << "\tmode: " << p.mode << std::endl
        << "\tphase: " << p.phase << std::endl
        << "\tround: " << p.round << std::endl
        << "\tct: " << p.teamCt << std::endl
        << "\tt: " << p.teamT << std::endl;

    for (size_t i = 0; i < p.roundWins.size(); ++i) {
        os << "\t R" << i + 1 << ": " << p.roundWins[i] << std::endl;
    }
    
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiWeaponPacket& p) {
    os << "CsgoGsiWeaponPacket {" << std::endl
        << "\tname: " << p.name << std::endl
        << "\tpaintkit: " << p.paintkit << std::endl
        << "\tstate: " << p.state << std::endl
        << "\ttype: " << p.type << std::endl
        << "\tclip: " << p.ammoClip << std::endl
        << "\tclip max: " << p.ammoClipMax << std::endl
        << "\treserve: " << p.ammoReserve << std::endl
        << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const game_event_watcher::CsgoGsiPlayerStatePacket& p) {
    os << "CsgoGsiPlayerStatePacket {" << std::endl
        << "\tarmor: " << p.armor << std::endl
        << "\tburning: " << p.burning << std::endl
        << "\tequipmentValue: " << p.equipmentValue << std::endl
        << "\tflashed: " << p.flashed << std::endl
        << "\thealth: " << p.health << std::endl
        << "\thelmet: " << p.helmet << std::endl
        << "\tmoney: " << p.money << std::endl
        << "\tround kill hs: " << p.roundKillsHs << std::endl
        << "\tround kills: " << p.roundKills << std::endl
        << "\tsmoked: " << p.smoked << std::endl
        << "\tdefuse kit: " << p.defuseKit << std::endl
        << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiPlayerMatchStatsPacket& p) {
    os << "CsgoGsiPlayerMatchStatsPacket {" << std::endl
        << "\tscore: " << p.score << std::endl
        << "\tmvps: " << p.mvps << std::endl
        << "\tkills: " << p.kills << std::endl
        << "\tdeaths: " << p.deaths << std::endl
        << "\tassists: " << p.assists << std::endl
        << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiPlayerPacket& p) {
    os << "CsgoGsiPlayerPacket {" << std::endl
        << "\tactivity: " << p.activity << std::endl
        << "\tname: " << p.name << std::endl
        << "\tsteamid: " << p.steamId << std::endl
        << "\tstats: " << p.matchStats << std::endl
        << "\tteam: " << p.team << std::endl;

    for (auto i = 0; i < p.weapons.size(); ++i) {
        os << "\tweapon[" << i << "]: " << p.weapons[i] << std::endl;
    }

    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiProviderPacket& p) {
    os << "CsgoGsiProviderPacket {" << std::endl
        << "\tappId: " << p.appId << std::endl
        << "\tname: " << p.name << std::endl
        << "\tsteamId: " << p.steamId << std::endl
        << "\ttimestamp: " << p.timestamp << std::endl
        << "\tversion: " << p.version << std::endl
        << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CsgoGsiRoundPacket& p) {
    os << "CsgoGsiRoundPacket {" << std::endl
        << "\tphase: " << p.phase << std::endl
        << "\tbomb: " << p.bomb << std::endl
        << "\twin team: " << p.winTeam << std::endl
        << "}";
    return os;
}