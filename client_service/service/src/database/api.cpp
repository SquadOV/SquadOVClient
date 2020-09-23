#include "database/api.h"

#include <stdexcept>
#include <sstream>
#include "shared/errors/error.h"
#include "shared/time.h"
#include "valorant/valorant_match.h"
#include "valorant/valorant_match_details.h"

#include <iostream>

namespace {

class Sqlite3ErrorMessage {
public:
    ~Sqlite3ErrorMessage() {
        sqlite3_free(_msg);
    }

    char** buffer() {
        return &_msg;
    }

    std::string str() const {
        return std::string(_msg);
    }

private:
    char* _msg = nullptr;
};

class SqlStatement {
public:
    SqlStatement(sqlite3* db, const std::string& statement):
        _db(db) {
        _errorCode = sqlite3_prepare(
            _db,
            statement.c_str(),
            static_cast<int>(statement.size()),
            &_stmt,
            NULL
        );

        next();
    }

    ~SqlStatement() {
        sqlite3_finalize(_stmt);
    }

    bool hasNext() const { return _hasNext; }

    void next() {
        if (!hasNext()) {
            return;
        }

        const int code = sqlite3_step(_stmt);
        _hasNext = (code != SQLITE_DONE);
        if (code != SQLITE_DONE && code != SQLITE_OK) {
            _errorCode = code;
        }
    }

    template<typename T>
    T getColumn(int i) {}

    template<>
    int getColumn(int i) {
        return sqlite3_column_int(_stmt, i);
    }

    bool fail() const { return errorCode() != SQLITE_OK; }
    int errorCode() const { return _errorCode; }
private:
    sqlite3* _db = nullptr;
    sqlite3_stmt* _stmt = nullptr;
    int _errorCode = 0;
    bool _hasNext = true;
};

}

namespace service::database {
namespace {

void addNullableTimeSql(std::ostringstream& sql, const shared::TimePoint& tm) {
    if (shared::isTimeValid(tm)) {
        sql << "'" << shared::timeToStr(tm, true) << "'";
    } else {
        sql << "null";
    }
}

void insertValorantTeamSql(std::ostringstream& sql, const std::string& matchId, const service::valorant::ValorantMatchTeam& team) {
     sql << R"|(
            INSERT INTO valorant_match_teams (
                match_id,
                team_id,
                won,
                rounds_won,
                rounds_played
            )
            VALUES ()|"
                << "'" << matchId << "', " 
                << "'" << team.teamId()  << "', " 
                << team.won() << ", " 
                << team.roundsWon() << ", "
                << team.roundsPlayed() << ");";
    sql << std::endl;
}

void insertValorantPlayerSql(std::ostringstream& sql, const std::string& matchId, const service::valorant::ValorantMatchPlayer& player) {
    sql << R"|(
            INSERT INTO valorant_match_players (
                match_id,
                team_id,
                puuid,
                agent_id,
                competitive_tier,
                total_combat_score,
                rounds_played,
                kills,
                deaths,
                assists
            )
            VALUES ()|"
                << "'" << matchId << "', " 
                << "'" << player.teamId()  << "', " 
                << "'" << player.puuid()  << "', "
                << "'" << player.agentId()  << "', "
                << player.competitiveTier() << ", " 
                << player.stats().totalCombatScore << ", "
                << player.stats().roundsPlayed << ", "
                << player.stats().kills << ", "
                << player.stats().deaths << ", "
                << player.stats().assists << ");";
    sql << std::endl;
}

void insertValorantLoadoutSql(std::ostringstream& sql, const std::string& matchId, const service::valorant::ValorantMatchLoadout& loadout) {
    sql << R"|(
            INSERT INTO valorant_match_round_player_loadout (
                match_id,
                round_num,
                puuid,
                loadout_value,
                remaining_money,
                spent_money,
                weapon,
                armor
            )
            VALUES ()|"
                << "'" << matchId << "', " 
                << loadout.roundNum() << ", " 
                << "'" << loadout.puuid()  << "', "
                << loadout.loadoutValue() << ", "
                << loadout.remainingMoney() << ", "
                << loadout.spentMoney() << ", "
                << "'" << loadout.weapon() << "', "
                << "'" << loadout.armor() << "');";
    sql << std::endl;
}

void insertValorantRoundSql(std::ostringstream& sql, const std::string& matchId, const service::valorant::ValorantMatchRound& round) {
    sql << R"|(
            INSERT INTO valorant_match_rounds (
                match_id,
                round_num,
                plant_round_time,
                planter_puuid,
                defuse_round_time,
                defuser_puuid,
                team_round_winner,
                round_buy_time_utc,
                round_play_time_utc
            )
            VALUES ()|"
                << "'" << matchId << "', " 
                << round.roundNum() << ", ";

    if (round.didPlant()) {
        sql << round.plantRoundTime() << ", '" << round.planterPuuid() << "'";
    } else {
        sql << "null, null";
    }
    sql << ", ";

    if (round.didDefuse()) {
        sql << round.defuseRoundTime() << ", '" << round.defuserPuuid() << "'";
    } else {
        sql << "null, null";
    }
    sql << ", "
        << "'" << round.teamWinner() << "', ";

    addNullableTimeSql(sql, round.startBuyTime());
    sql << ", ";

    addNullableTimeSql(sql, round.startPlayTime());
    sql << "); ";
    sql << std::endl;

    for (const auto& [puuid, loadout] : round.playerLoadouts()) {
        insertValorantLoadoutSql(sql, matchId, *loadout);
    }

    for (const auto& [puuid, score] : round.roundCombatScore()) {
        sql << R"|(
            INSERT INTO valorant_match_round_player_stats (
                match_id,
                round_num,
                puuid,
                combat_score
            )
            VALUES ()|"
                << "'" << matchId << "', " 
                << round.roundNum() << ", "
                << "'" << puuid << "', "
                << score << ");";
        sql << std::endl;
    }
}

void insertValorantKillSql(std::ostringstream& sql, const std::string& matchId, const service::valorant::ValorantMatchKill& kill) {
    sql << R"|(
            INSERT INTO valorant_match_kill (
                match_id,
                round_num,
                killer_puuid,
                victim_puuid,
                round_time,
                damage_type,
                damage_item,
                is_secondary_fire
            )
            VALUES ()|"
                << "'" << matchId << "'," 
                << kill.round() << ", "
                << "'" << kill.killerPuuid() << "', "
                << "'" << kill.victimPuuid() << "', "
                << kill.roundTime() << ", "
                << "'" << kill.damageType() << "', "
                << "'" << kill.damageItem() << "', "
                << (int)kill.killSecondaryFire() << ");";
    sql << std::endl;
}

void insertValorantDamageSql(std::ostringstream& sql, const std::string& matchId, const service::valorant::ValorantMatchDamage& dmg) {
    sql << R"|(
            INSERT INTO valorant_match_damage (
                match_id,
                round_num,
                instigator_puuid,
                receiver_puuid,
                damage,
                legshots,
                bodyshots,
                headshots
            )
            VALUES ()|"
                << "'" << matchId << "'," 
                << dmg.roundNum() << ", "
                << "'" << dmg.instigatorPuuid() << "', "
                << "'" << dmg.receiverPuuid() << "', "
                << dmg.damage() << ", "
                << dmg.legshots() << ", "
                << dmg.bodyshots() << ", "
                << dmg.headshots() << ");";
    sql << std::endl;
}

void insertValorantMatchSql(std::ostringstream& sql, const service::valorant::ValorantMatch* match) {
    const auto& details = match->details();
    sql << R"|(
            INSERT INTO valorant_matches (
                id,
                gameMode,
                map,
                is_ranked,
                custom_game_name,
                game_version,
                server_start_time_utc,
                start_time_utc,
                end_time_utc
            )
            VALUES ()|"
                << "'" << match->matchId() << "', " 
                << "'" << details.gameMode() << "', " 
                << "'" << details.map() << "', "
                << (int)details.isRanked() << ", " 
                << "'" << details.customGameName() << "', "
                << "'" << details.gameVersion() << "', "
                << details.startTime() << ", ";
    addNullableTimeSql(sql, match->startTime());
    sql << ", ";
    addNullableTimeSql(sql, match->endTime());
    sql << ");" << std::endl;

    for (const auto& [teamId, team] : details.teams()) {
        insertValorantTeamSql(sql, match->matchId(), *team);
    }

    for (const auto& [puuid, player] : details.players()) {
        insertValorantPlayerSql(sql, match->matchId(), *player);
    }

    for (const auto& round : details.rounds()) {
        insertValorantRoundSql(sql, match->matchId(), *round);
    }

    for (const auto& kill : details.kills()) {
        insertValorantKillSql(sql, match->matchId(), *kill);
    }

    for (const auto& dmg : details.damage()) {
        insertValorantDamageSql(sql, match->matchId(), *dmg);
    }
}

}

DatabaseApi::DatabaseApi(const std::string& dbFname) {
    if (sqlite3_open(dbFname.c_str(), &_db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database.");
    }
}

DatabaseApi::~DatabaseApi() {
    sqlite3_close(_db);
}

void DatabaseApi::storeValorantAccount(const shared::riot::RiotUser& user) const {
    std::ostringstream sql;
    sql << "BEGIN EXCLUSIVE TRANSACTION;"
        << "INSERT INTO valorant_accounts (puuid, username, tag) \
            VALUES ('" << user.puuid << "', '" << user.username << "', '" << user.tag << "') \
            ON CONFLICT DO NOTHING;"
        << "COMMIT TRANSACTION;";

    Sqlite3ErrorMessage errMsg;
    if (sqlite3_exec(_db, sql.str().c_str(), nullptr, nullptr, errMsg.buffer()) != SQLITE_OK) {
        THROW_ERROR("Failed to store valorant account: " << errMsg.str());
    }
}

void DatabaseApi::storeValorantMatch(const service::valorant::ValorantMatch* match) const {
    std::ostringstream sql;
    sql << "BEGIN EXCLUSIVE TRANSACTION;";
    insertValorantMatchSql(sql, match);
    sql << "COMMIT TRANSACTION;";

    Sqlite3ErrorMessage errMsg;
    if (sqlite3_exec(_db, sql.str().c_str(), nullptr, nullptr, errMsg.buffer()) != SQLITE_OK) {
        THROW_ERROR("Failed to store valorant match: " << errMsg.str());
    }
}

size_t DatabaseApi::totalValorantMatchesForPuuid(const std::string& puuid) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT COUNT(vm.id)
        FROM valorant_matches AS vm
        INNER JOIN valorant_match_players AS vmp
            ON vmp.match_id = vm.id
        WHERE vmp.puuid = ')|" << puuid << "';";

    SqlStatement stmt(_db, sql.str());
    if (!stmt.hasNext()) {
        return false;
    }
    return static_cast<size_t>(stmt.getColumn<int>(0));
}

bool DatabaseApi::isValorantMatchStored(const std::string& matchId) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT COUNT(vm.id)
        FROM valorant_matches AS vm
        WHERE vm.id = ')|" << matchId << "';";

    SqlStatement stmt(_db, sql.str());
    if (!stmt.hasNext()) {
        return false;
    }

    return (stmt.getColumn<int>(0) > 0);
}

}