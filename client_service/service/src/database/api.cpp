#include "database/api.h"

#include <stdexcept>
#include <sstream>
#include "shared/errors/error.h"
#include "shared/time.h"
#include "valorant/valorant_match.h"
#include "valorant/valorant_match_details.h"

#include <iostream>

namespace {

class SqlStatement {
public:
    SqlStatement(sqlite3* db, const std::string& statement):
        _ogStatement(statement),
        _db(db) {
        _errorCode = sqlite3_prepare(
            _db,
            statement.c_str(),
            static_cast<int>(statement.size()),
            &_stmt,
            NULL
        );
        _errMsg = sqlite3_errmsg(_db);
    }

    virtual ~SqlStatement() {
        sqlite3_finalize(_stmt);
    }

    SqlStatement(const SqlStatement&) = delete;
    SqlStatement& operator=(const SqlStatement&) = delete;

    SqlStatement(SqlStatement&&) = default;
    SqlStatement& operator=(SqlStatement&&) = default;

    bool hasNext() const { return _hasNext && !fail(); }

    virtual void next() {
        if (!hasNext()) {
            return;
        }

        const int code = sqlite3_step(_stmt);
        _hasNext = (code == SQLITE_ROW);
        if (code != SQLITE_DONE && code != SQLITE_OK && code != SQLITE_ROW) {
            _errorCode = code;
            _errMsg = sqlite3_errmsg(_db);
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
    std::string errMsg() const { 
        std::ostringstream str;
        str << _errMsg << "[" << _errorCode << " - " << sqlite3_errstr(_errorCode) << "]" << std::endl
            << "\tStmt: " << _ogStatement;
        return str.str();
    }

protected:
    sqlite3* _db = nullptr;

private:
    std::string _ogStatement;
    sqlite3_stmt* _stmt = nullptr;
    int _errorCode = SQLITE_OK;
    bool _hasNext = true;
    std::string _errMsg;
};

class SqlTransaction {
public:
    SqlTransaction(sqlite3* db):
        _db(db) {
        SqlStatement begin(_db, "BEGIN IMMEDIATE TRANSACTION;");
        begin.next();
    }

    void exec(SqlStatement& stmt) {
        if (!_commit) {
            return;
        }
        stmt.next();
        _commit = !stmt.fail();
        if (!_commit) {
            _error = stmt.errMsg();
        }
    }

    bool fail() const { return !_commit; }
    std::string errMsg() const { return _error; }

    ~SqlTransaction() {
        std::string endStmt;
        if (!_commit) {
            endStmt = "ROLLBACK;";
        } else {
            endStmt = "COMMIT;";
        }

        SqlStatement end(_db, endStmt);
        end.next();
    }

    SqlStatement createStatement(const std::string& str) {
        return SqlStatement(_db, str);
    }

private:
    sqlite3* _db = nullptr;
    bool _commit = true;
    std::string _error;

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

void insertValorantTeamSql(SqlTransaction& tx, const std::string& matchId, const std::unordered_map<std::string, service::valorant::ValorantMatchTeamPtr>& teams) {
    if (teams.empty()) {
        return;
    }

    std::ostringstream sql;
    sql << R"|(
        INSERT INTO valorant_match_teams (
            match_id,
            team_id,
            won,
            rounds_won,
            rounds_played
        )
        VALUES )|";

    size_t count = 0;
    for (const auto& [teamId, team] : teams) {
        sql << "("
            << "'" << matchId << "', " 
            << "'" << team->teamId()  << "', " 
            << team->won() << ", " 
            << team->roundsWon() << ", "
            << team->roundsPlayed() << ")";

        sql << ((++count == teams.size()) ? ";" : ", ");
    }

    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);
}

void insertValorantPlayerSql(SqlTransaction& tx, const std::string& matchId, const std::unordered_map<std::string, service::valorant::ValorantMatchPlayerPtr>& players) {
    if (players.empty()) {
        return;
    }

    std::ostringstream sql;
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
            VALUES )|";

    size_t count = 0;
    for (const auto& [puuid, player] : players) {
        sql << "('" << matchId << "', " 
            << "'" << player->teamId()  << "', " 
            << "'" << player->puuid()  << "', "
            << "'" << player->agentId()  << "', "
            << player->competitiveTier() << ", " 
            << player->stats().totalCombatScore << ", "
            << player->stats().roundsPlayed << ", "
            << player->stats().kills << ", "
            << player->stats().deaths << ", "
            << player->stats().assists << ")";

        sql << ((++count == players.size()) ? ";" : ", ");
    }

    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);
}

void insertValorantLoadoutSql(SqlTransaction& tx, const std::string& matchId, const std::unordered_map<std::string, service::valorant::ValorantMatchLoadoutPtr>& loadouts) {
    if (loadouts.empty()) {
        return;
    }

    std::ostringstream sql;
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
            VALUES )|";
    
    size_t count = 0;
    for (const auto& [puuid, loadout] : loadouts) {
        sql << "('" << matchId << "', " 
            << loadout->roundNum() << ", " 
            << "'" << loadout->puuid()  << "', "
            << loadout->loadoutValue() << ", "
            << loadout->remainingMoney() << ", "
            << loadout->spentMoney() << ", "
            << "'" << loadout->weapon() << "', "
            << "'" << loadout->armor() << "')";
        sql << ((++count == loadouts.size()) ? ";" : ", ");
    }

    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);
}

void insertValorantRoundPlayerStats(SqlTransaction& tx, const std::string& matchId, int roundNum, const std::unordered_map<std::string, int>& stats) {
    if (stats.empty()) {
        return;
    }

    std::ostringstream sql;
    sql << R"|(
        INSERT INTO valorant_match_round_player_stats (
            match_id,
            round_num,
            puuid,
            combat_score
        )
        VALUES )|";
    size_t count = 0;
    for (const auto& [puuid, score] : stats) {
        sql << "('" << matchId << "', " 
            << roundNum << ", "
            << "'" << puuid << "', "
            << score << ")";
        sql << ((++count == stats.size()) ? ";" : ", ");
    }

    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);
}

void insertValorantRoundSql(SqlTransaction& tx, const std::string& matchId, const std::vector<service::valorant::ValorantMatchRoundPtr>& rounds) {
    if (rounds.empty()) {
        return;
    }

    std::ostringstream sql;
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
            VALUES )|";

    size_t count = 0;
    for (const auto& round : rounds) {
        sql << "('" << matchId << "', " 
            << round->roundNum() << ", ";

        if (round->didPlant()) {
            sql << round->plantRoundTime() << ", '" << round->planterPuuid() << "'";
        } else {
            sql << "null, null";
        }
        sql << ", ";

        if (round->didDefuse()) {
            sql << round->defuseRoundTime() << ", '" << round->defuserPuuid() << "'";
        } else {
            sql << "null, null";
        }
        sql << ", "
            << "'" << round->teamWinner() << "', ";

        addNullableTimeSql(sql, round->startBuyTime());
        sql << ", ";

        addNullableTimeSql(sql, round->startPlayTime());
        sql << ")";

        sql << ((++count == rounds.size()) ? ";" : ", ");
    }
    
    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);

    if (tx.fail()) {
        return;
    }

    for (const auto& round : rounds) {
        insertValorantLoadoutSql(tx, matchId, round->playerLoadouts());
        if (tx.fail()) {
            return;
        }

        insertValorantRoundPlayerStats(tx, matchId, round->roundNum(), round->roundCombatScore());
        if (tx.fail()) {
            return;
        }
    }
}

void insertValorantKillSql(SqlTransaction& tx, const std::string& matchId, const std::vector<service::valorant::ValorantMatchKillPtr>& kills) {
    if (kills.empty()) {
        return;
    }

    std::ostringstream sql;
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
            VALUES )|";
    
    size_t count = 0;
    for (const auto& kill : kills) {
        sql << "('" << matchId << "'," 
            << kill->round() << ", "
            << "'" << kill->killerPuuid() << "', "
            << "'" << kill->victimPuuid() << "', "
            << kill->roundTime() << ", "
            << "'" << kill->damageType() << "', "
            << "'" << kill->damageItem() << "', "
            << (int)kill->killSecondaryFire() << ")";
        sql << ((++count == kills.size()) ? ";" : ", ");
    }
    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);
}

void insertValorantDamageSql(SqlTransaction& tx, const std::string& matchId,  const std::vector<service::valorant::ValorantMatchDamagePtr>& damage) {
    if (damage.empty()) {
        return;
    }

    std::ostringstream sql;
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
            VALUES )|";
    
    size_t count = 0;
    for (const auto& dmg : damage) {
        sql << "('" << matchId << "'," 
            << dmg->roundNum() << ", "
            << "'" << dmg->instigatorPuuid() << "', "
            << "'" << dmg->receiverPuuid() << "', "
            << dmg->damage() << ", "
            << dmg->legshots() << ", "
            << dmg->bodyshots() << ", "
            << dmg->headshots() << ")";
        sql << ((++count == damage.size()) ? ";" : ", ");
    }

    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);
}

void insertValorantMatchSql(SqlTransaction& tx, const service::valorant::ValorantMatch* match) {
    const auto& details = match->details();

    std::ostringstream sql;
    sql << R"|(
            INSERT INTO valorant_matches (
                id,
                gameMode,
                map,
                is_ranked,
                provisioning_flow_id,
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
                << "'" << details.provisioningFlowID() << "', "
                << "'" << details.gameVersion() << "', "
                << details.startTime() << ", ";
    addNullableTimeSql(sql, match->startTime());
    sql << ", ";
    addNullableTimeSql(sql, match->endTime());
    sql << ");" << std::endl;

    SqlStatement stmt = tx.createStatement(sql.str());
    tx.exec(stmt);

    if (tx.fail()) {
        return;
    }

    insertValorantTeamSql(tx, match->matchId(), details.teams());
    if (tx.fail()) {
        return;
    }

    insertValorantPlayerSql(tx, match->matchId(), details.players());
    if (tx.fail()) {
        return;
    }

    insertValorantRoundSql(tx, match->matchId(), details.rounds());
    if (tx.fail()) {
        return;
    }

    insertValorantKillSql(tx, match->matchId(), details.kills());
    if (tx.fail()) {
        return;
    }

    insertValorantDamageSql(tx, match->matchId(), details.damage());
    if (tx.fail()) {
        return;
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
    sql << "INSERT INTO valorant_accounts (puuid, username, tag) \
            VALUES ('" << user.puuid << "', '" << user.username << "', '" << user.tag << "') \
            ON CONFLICT DO NOTHING;";

    SqlTransaction tx(_db);
    SqlStatement stmt(_db, sql.str());
    tx.exec(stmt);

    if (stmt.fail()) {
        THROW_ERROR("Failed to store valorant account: " << stmt.errMsg());
    }
}

void DatabaseApi::storeValorantMatch(const service::valorant::ValorantMatch* match) const {
    SqlTransaction tx(_db);
    insertValorantMatchSql(tx, match);

    if (tx.fail()) {
        THROW_ERROR("Failed to store valorant match: " << tx.errMsg());
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
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get total valorant matches: " << stmt.errMsg());
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
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get if match stored: " << stmt.errMsg());
    }

    return (stmt.getColumn<int>(0) > 0);
}

}