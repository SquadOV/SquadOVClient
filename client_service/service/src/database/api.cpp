#include "database/api.h"

#include <stdexcept>
#include <sstream>
#include "shared/errors/error.h"
#include "shared/time.h"
#include "shared/sqlite/sqlite.h"
#include "valorant/valorant_match.h"
#include "valorant/valorant_match_details.h"

#include <iostream>

using namespace shared::sqlite;

namespace service::database {
namespace {

void addNullableTimeSql(std::ostringstream& sql, const shared::TimePoint& tm) {
    if (shared::isTimeValid(tm)) {
        sql << "'" << shared::timeToStr(tm, true) << "'";
    } else {
        sql << "null";
    }
}

void addNullableString(std::ostringstream& sql, const std::string& str) {
    if (!str.empty()) {
        sql << "'" << str << "'";
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

void DatabaseApi::associateValorantMatchToVideoFile(const std::string& matchId, const std::string& fname) const {
    std::ostringstream sql;
    sql << "INSERT INTO valorant_match_videos (match_id, video_path) \
            VALUES ('" << matchId << "', '" << fname  << "') \
            ON CONFLICT DO NOTHING;";

    SqlTransaction tx(_db);
    SqlStatement stmt(_db, sql.str());
    tx.exec(stmt);

    if (stmt.fail()) {
        THROW_ERROR("Failed to store valorant account: " << stmt.errMsg());
    }
}

bool DatabaseApi::isValorantVideoAssociatedWithMatch(const std::string& fname) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT COUNT(vmv.match_id)
        FROM valorant_match_videos AS vmv
        WHERE vmv.video_path = ')|" << fname << "';";

    SqlStatement stmt(_db, sql.str());
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get if video is assocaited with match: " << stmt.errMsg());
    }

    return (stmt.getColumn<int>(0) > 0);
}

std::vector<std::string> DatabaseApi::getMatchIdsForPlayer(const std::string& puuid) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT match_id
        FROM valorant_match_players
        WHERE puuid = ')|" << puuid << "';";

    std::vector<std::string> ret;
    SqlStatement stmt(_db, sql.str());

    while (stmt.hasNext()) {
        if (!stmt.next()) {
            break;
        }

        if (stmt.fail()) {
            THROW_ERROR("Failed to get match ids: " << stmt.errMsg());
        }

        ret.push_back(stmt.getColumn<std::string>(0));
    }
    return ret;
}

void DatabaseApi::storeAimlabTask(const shared::aimlab::TaskData& task, const std::string& vodPath) const {
    std::ostringstream sql;
    sql << R"|(
        INSERT INTO aimlab_tasks (
            id,
            taskName,
            mode,
            score,
            createDate,
            version,
            rawData,
            vodPath
        )
        VALUES (
            ?,
            ?,
            ?,
            ?,
            ?,
            ?,
            ?,
            ?
        ))|";

    SqlTransaction tx(_db);

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, task.taskId);
    stmt.bindParameter(2, task.taskName);
    stmt.bindParameter(3, task.mode);
    stmt.bindParameter(4, task.score);
    stmt.bindParameter(5, shared::timeToStr(task.createDate, true));
    stmt.bindParameter(6, task.version);
    stmt.bindParameter(7, task.rawData);
    if (vodPath.empty()) {
        stmt.bindNullParameter(8);
    } else {
        stmt.bindParameter(8, vodPath);
    }

    tx.exec(stmt);

    if (stmt.fail()) {
        THROW_ERROR("Failed to store Aim Lab task: " << stmt.errMsg());
    }
}

bool DatabaseApi::isAimlabVideoAssociatedWithTask(const std::string& fname) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT COUNT(id)
        FROM aimlab_tasks
        WHERE vodPath = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, fname);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get if video is assocaited with match: " << stmt.errMsg());
    }

    return (stmt.getColumn<int>(0) > 0);
}

std::vector<int> DatabaseApi::allStoredAimlabTaskIds() const {
    std::ostringstream sql;
    sql << R"|(
        SELECT id
        FROM aimlab_tasks
        ORDER BY id DESC)|";

    std::vector<int> ret;
    SqlStatement stmt(_db, sql.str());

    while (stmt.hasNext()) {
        if (!stmt.next()) {
            break;
        }

        if (stmt.fail()) {
            THROW_ERROR("Failed to get task ids: " << stmt.errMsg());
        }

        ret.push_back(stmt.getColumn<int>(0));
    }
    return ret;
}

}