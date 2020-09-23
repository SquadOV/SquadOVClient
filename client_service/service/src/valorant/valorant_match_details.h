#pragma once

#include "shared/strings/strings.h"
#include "shared/time.h"
#include "shared/valorant/valorant.h"

#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include <ostream>
#include <unordered_map>
#include <vector>

// We don't parse the data into an enum format just in case we can't update the app on time.
// In that case, we want to collect as much as the raw data as possible and work on accurately reading it at a later time.

namespace service::valorant {

class ValorantMatchPlayer;
class ValorantMatchTeam;
class ValorantMatchRound;
class ValorantMatchKill;
class ValorantMatchDamage;
class ValorantMatchLoadout;

class ValorantMatchTeam {
public:
    const std::string& teamId() const { return _teamId; }
    bool won() const { return _won; }
    int roundsWon() const { return _roundsWon; }
    int roundsPlayed() const { return _roundsPlayed; }

    friend class ValorantMatchDetails;
private:
    // "teamId"
    std::string _teamId;

    // "won"
    bool _won = false;

    // "roundsWon"
    int _roundsWon = 0;
    int _roundsPlayed = 0;

    std::vector<const ValorantMatchPlayer*> _players;
};
using ValorantMatchTeamPtr = std::unique_ptr<ValorantMatchTeam>;


class ValorantMatchPlayer {
public:
    struct PlayerStats {
        int totalCombatScore = 0;
        int roundsPlayed = 0;
        int kills = 0;
        int deaths = 0;
        int assists = 0;
    };
    const std::string& teamId() const { return _team->teamId(); }
    const std::string& puuid() const { return _puuid; }
    const std::string& agentId() const { return _characterId; }
    const PlayerStats& stats() const { return _stats; }
    int competitiveTier() const { return _competitiveTier; }

    friend class ValorantMatchDetails;
    
private:
    // "subject"
    std::string _puuid;

    // "characterId"
    std::string _characterId;

    // "stats"
    PlayerStats _stats;

    // "competitiveTier"
    int _competitiveTier = 0;

    const ValorantMatchTeam* _team = nullptr;
    std::vector<const ValorantMatchKill*> _kills;
    std::vector<const ValorantMatchKill*> _deaths;
    std::vector<const ValorantMatchDamage*> _damageDone;
};
using ValorantMatchPlayerPtr = std::unique_ptr<ValorantMatchPlayer>;


class ValorantMatchLoadout {
public:
    int loadoutValue() const { return _loadoutValue; }
    int remainingMoney() const { return _remainingMoney; }
    int spentMoney() const { return _spentMoney; }
    const std::string& weapon() const { return _weapon; }
    const std::string& armor() const { return _armor; }
    int roundNum() const { return _roundNum; }
    const std::string& puuid() const { return _owner->puuid(); }

    friend class ValorantMatchDetails;
private:
    int _loadoutValue = 0;
    int _remainingMoney = 0;
    int _spentMoney = 0;
    std::string _weapon;
    std::string _armor;

    const ValorantMatchPlayer* _owner = nullptr;
    int _roundNum = 0;
};
using ValorantMatchLoadoutPtr = std::unique_ptr<ValorantMatchLoadout>;

class ValorantMatchRound {
public:
    int roundNum() const { return _roundNum; }

    bool didPlant() const { return _plantRoundTime > 0; }
    int plantRoundTime() const { return _plantRoundTime; }
    const std::string& planterPuuid() const { return !!_planter ? _planter->puuid() : shared::strings::cEmptyString; }

    bool didDefuse() const { return _defuseRoundTime > 0; }
    int defuseRoundTime() const { return _defuseRoundTime; }
    const std::string& defuserPuuid() const { return !!_defuser ? _defuser->puuid() : shared::strings::cEmptyString; }

    const std::string& teamWinner() const { return _roundWinner; }

    const shared::TimePoint& startBuyTime() const { return _startBuyTime; }
    const shared::TimePoint& startPlayTime() const { return _startPlayTime; }

    const auto& playerLoadouts() const { return _playerLoadouts; }
    const auto& roundCombatScore() const { return _roundCombatScore; }

    friend class ValorantMatchDetails;
private:
    shared::TimePoint _startBuyTime = shared::TimePoint::max();
    shared::TimePoint _startPlayTime = shared::TimePoint::max();

    // "roundNum"
    int _roundNum = 0;

    int _plantRoundTime = 0;
    const ValorantMatchPlayer* _planter = nullptr;

    int _defuseRoundTime = 0;
    const ValorantMatchPlayer* _defuser = nullptr;
    
    // "winningTeam"
    std::string _roundWinner;

    std::unordered_map<std::string, ValorantMatchLoadoutPtr> _playerLoadouts;
    std::unordered_map<std::string, int> _roundCombatScore;

    std::vector<const ValorantMatchKill*> _kills;
    std::vector<const ValorantMatchDamage*> _damage;
};
using ValorantMatchRoundPtr = std::unique_ptr<ValorantMatchRound>;

class ValorantMatchKill {
public:
    int roundTime() const { return _roundTime; }
    int round() const { return _round; }
    const std::string& damageType() const { return _damageType; }
    const std::string& damageItem() const { return _damageItem; }
    bool killSecondaryFire() const { return _killSecondaryFire; }
    const std::string& killerPuuid() const { return _killer->puuid(); }
    const std::string& victimPuuid() const { return _victim->puuid(); }

    friend class ValorantMatchDetails;
private:
    const ValorantMatchPlayer* _killer;
    const ValorantMatchPlayer* _victim;

    int _roundTime = 0;
    int _round = 0;
    std::string _damageType;
    std::string _damageItem;
    bool _killSecondaryFire = false;
};
using ValorantMatchKillPtr = std::unique_ptr<ValorantMatchKill>;

class ValorantMatchDamage {
public:
    int roundNum() const { return _roundNum; }
    int damage() const { return _damage; }
    int legshots() const { return _legshots; }
    int bodyshots() const { return _bodyshots; }
    int headshots() const { return _headshots; }

    const std::string& instigatorPuuid() const { return _instigator->puuid(); }
    const std::string& receiverPuuid() const { return _receiver->puuid(); }

    friend class ValorantMatchDetails;
private:
    const ValorantMatchPlayer* _instigator;
    const ValorantMatchPlayer* _receiver;

    int _roundNum = 0;
    int _damage = 0;
    int _legshots = 0;
    int _bodyshots = 0;
    int _headshots = 0;
};
using ValorantMatchDamagePtr = std::unique_ptr<ValorantMatchDamage>;

class ValorantMatchDetails {
public:
    void parseFromJson(const std::string& json);
    void nextRoundStateWithTimestamp(const shared::TimePoint& tm, shared::valorant::EValorantRoundState state);

    // Assuming the current ValorantMatchDetails is one created
    // using the local logs and the other ValorantMatchDetails is
    // one pulled from the API. This will leave the other match
    // detail object in an undefined state.
    void mergeWithApi(ValorantMatchDetails* other);

    const std::string& gameMode() const { return _gameMode; }
    const std::string& map() const { return _map; }
    bool isRanked() const { return _isRanked; }
    const std::string& provisioningFlowID() const { return _provisioningFlowID; }
    const std::string& gameVersion() const { return _gameVersion; }
    const long long startTime() const { return _startTime; }
    const std::string& matchId() const { return _matchId; }

    const std::unordered_map<std::string, ValorantMatchPlayerPtr>& players() const { return _players; }
    const std::unordered_map<std::string, ValorantMatchTeamPtr>& teams() const { return _teams; }
    const std::vector<ValorantMatchRoundPtr>& rounds() const { return _rounds; }
    const std::vector<ValorantMatchKillPtr>& kills() const { return _kills; }
    const std::vector<ValorantMatchDamagePtr>& damage() const { return _damage; }

private:
    void parseMatchInfoJson(const nlohmann::json& obj);
    void parsePlayersJson(const nlohmann::json& obj);
    void parseTeamsJson(const nlohmann::json& obj);
    void parseRoundsJson(const nlohmann::json& obj);
    void parseKillsJson(const nlohmann::json& obj);

    std::string _gameMode;
    std::string _map;
    bool _isRanked = false;
    std::string _provisioningFlowID;
    std::string _gameVersion;
    long long _startTime = 0;
    std::string _matchId;

    // puuid to player
    std::unordered_map<std::string, ValorantMatchPlayerPtr> _players;
    std::unordered_map<std::string, ValorantMatchTeamPtr> _teams;

    std::vector<ValorantMatchRoundPtr> _rounds;
    std::vector<ValorantMatchKillPtr> _kills;
    std::vector<ValorantMatchDamagePtr> _damage;
};
using ValorantMatchDetailsPtr = std::unique_ptr<ValorantMatchDetails>;

std::ostream& operator<<(std::ostream& out, const ValorantMatchDetails& details);
std::ostream& operator<<(std::ostream& out, const ValorantMatchDamage& dmg);
std::ostream& operator<<(std::ostream& out, const ValorantMatchKill& kill);
std::ostream& operator<<(std::ostream& out, const ValorantMatchRound& round);
std::ostream& operator<<(std::ostream& out, const ValorantMatchPlayer& player);
std::ostream& operator<<(std::ostream& out, const ValorantMatchTeam& team);

}