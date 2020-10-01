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
    void setTeamId(const std::string& s) { _teamId = s ;}

    bool won() const { return _won; }
    void setWon(bool w) { _won = w; }

    int roundsWon() const { return _roundsWon; }
    void setRoundsWon(int w) { _roundsWon = w; }

    int roundsPlayed() const { return _roundsPlayed; }
    void setRoundsPlayed(int p) { _roundsPlayed = p; }

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
    void setTeam(const ValorantMatchTeam* team) { _team = team; } 

    const std::string& puuid() const { return _puuid; }
    void setPuuid(const std::string& puuid) { _puuid = puuid; }

    const std::string& agentId() const { return _characterId; }
    void setAgentId(const std::string& agentId) { _characterId = agentId; }

    const PlayerStats& stats() const { return _stats; }
    void setStatCombatScore(int s) { _stats.totalCombatScore = s; }
    void setStatRoundsPlayed(int s) { _stats.roundsPlayed = s; }
    void setStatKills(int s) { _stats.kills = s; }
    void setStatDeaths(int s) { _stats.deaths = s; }
    void setStatAssists(int s) { _stats.assists = s; }

    int competitiveTier() const { return _competitiveTier; }
    void setCompetitiveTier(int s) { _competitiveTier = s; }

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
    void setLoadoutValue(int v) { _loadoutValue = v; }

    int remainingMoney() const { return _remainingMoney; }
    void setRemainingMoney(int v) { _remainingMoney = v; }

    int spentMoney() const { return _spentMoney; }
    void setSpentMoney(int v) { _spentMoney = v; }

    const std::string& weapon() const { return _weapon; }
    void setWeapon(const std::string& w) { _weapon = w; }

    const std::string& armor() const { return _armor; }
    void setArmor(const std::string& w) { _armor = w; }
    
    int roundNum() const { return _roundNum; }
    void setRoundNum(int r) { _roundNum = r; }

    const std::string& puuid() const { return _owner->puuid(); }
    void setOwner(const ValorantMatchPlayer* owner) { _owner = owner; }

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
    void setRoundNum(int i) { _roundNum = i; }

    bool didPlant() const { return _plantRoundTime > 0; }

    int plantRoundTime() const { return _plantRoundTime; }
    void setPlantTime(int i) { _plantRoundTime = i; }

    const std::string& planterPuuid() const { return !!_planter ? _planter->puuid() : shared::strings::cEmptyString; }
    void setPlanter(const ValorantMatchPlayer* p) { _planter = p; }

    bool didDefuse() const { return _defuseRoundTime > 0; }

    int defuseRoundTime() const { return _defuseRoundTime; }
    void setDefuseTime(int i) { _defuseRoundTime = i; }

    const std::string& defuserPuuid() const { return !!_defuser ? _defuser->puuid() : shared::strings::cEmptyString; }
    void setDefuser(const ValorantMatchPlayer* p) { _defuser = p; }

    const std::string& teamWinner() const { return _roundWinner; }
    void setTeamWinner(const std::string& w) { _roundWinner = w; }

    const shared::TimePoint& startBuyTime() const { return _startBuyTime; }
    void setStartBuyTime(const shared::TimePoint& p) { _startBuyTime = p; }

    const shared::TimePoint& startPlayTime() const { return _startPlayTime; }
    void setStartPlayTime(const shared::TimePoint& p) { _startPlayTime = p; }

    shared::valorant::EValorantRoundState currentRoundState() const;

    const auto& playerLoadouts() const { return _playerLoadouts; }
    void setLoadouts(std::vector<ValorantMatchLoadoutPtr>&& loadouts);

    const auto& roundCombatScore() const { return _roundCombatScore; }
    void setCombatScores(const std::unordered_map<std::string, int>& cs) { _roundCombatScore = cs; }

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
    void setRoundTime(int v) { _roundTime = v; }

    int round() const { return _round; }
    void setRound(int v) { _round = v; }

    const std::string& damageType() const { return _damageType; }
    void setDamageType(const std::string& v) { _damageType = v; }

    const std::string& damageItem() const { return _damageItem; }
    void setDamageItem(const std::string& v) { _damageItem = v; }

    bool killSecondaryFire() const { return _killSecondaryFire; }
    void setKillSecondaryFire(bool v) { _killSecondaryFire = v; }

    const std::string& killerPuuid() const { return _killer->puuid(); }
    void setKiller(const ValorantMatchPlayer* p) { _killer = p; }

    const std::string& victimPuuid() const { return _victim->puuid(); }
    void setVictim(const ValorantMatchPlayer* p) { _victim = p; }

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
    void setRoundNum(int v) { _roundNum = v; }

    int damage() const { return _damage; }
    void setDamage(int v) { _damage = v; }

    int legshots() const { return _legshots; }
    void setLegshots(int v) { _legshots = v; }

    int bodyshots() const { return _bodyshots; }
    void setBodyshots(int v) { _bodyshots = v; }

    int headshots() const { return _headshots; }
    void setHeadshots(int v) { _headshots = v; }

    const std::string& instigatorPuuid() const { return _instigator->puuid(); }
    void setInstigator(const ValorantMatchPlayer* v) { _instigator = v; }

    const std::string& receiverPuuid() const { return _receiver->puuid(); }
    void setReceiver(const ValorantMatchPlayer* v) { _receiver = v; }

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
    void setGameMode(const std::string& s) { _gameMode = s; }

    const std::string& map() const { return _map; }
    void setMap(const std::string& m) { _map = m; }

    bool isRanked() const { return _isRanked; }
    void setIsRanked(bool b) { _isRanked = b; }

    const std::string& provisioningFlowID() const { return _provisioningFlowID; }
    void setProvisioningFlowID(const std::string& s) { _provisioningFlowID = s; }

    const std::string& gameVersion() const { return _gameVersion; }
    void setGameVersion(const std::string& v) { _gameVersion = v; }

    const long long startTime() const { return _startTime; }
    void setStartTime(long long s) { _startTime = s; }

    const std::string& matchId() const { return _matchId; }
    void setMatchId(const std::string& m) { _matchId = m; }

    const std::string& rawApiData() const { return _rawApiData; }
    void setRawApiData(const std::string& m) { _rawApiData = m; }

    const std::unordered_map<std::string, ValorantMatchPlayerPtr>& players() const { return _players; }
    void setPlayers(std::vector<ValorantMatchPlayerPtr>& p);

    const std::unordered_map<std::string, ValorantMatchTeamPtr>& teams() const { return _teams; }
    void setTeams(std::vector<ValorantMatchTeamPtr>& t);

    const std::vector<ValorantMatchRoundPtr>& rounds() const { return _rounds; }
    void setRounds(std::vector<ValorantMatchRoundPtr>&& r);

    const std::vector<ValorantMatchKillPtr>& kills() const { return _kills; }
    void setKills(std::vector<ValorantMatchKillPtr>&& k);

    const std::vector<ValorantMatchDamagePtr>& damage() const { return _damage; }
    void setDamage(std::vector<ValorantMatchDamagePtr>&& d);

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
    std::string _rawApiData;

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