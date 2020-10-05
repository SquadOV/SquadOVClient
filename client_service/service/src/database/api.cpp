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
        sql << "'" << shared::timeToStr(tm) << "'";
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
        sql << ", ";
        addNullableString(sql, round->teamWinner());
        sql << ", ";
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
                end_time_utc,
                raw_api_data
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
    sql << "," << std::endl;
    addNullableString(sql, details.rawApiData());
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
        THROW_ERROR("Failed to open database.");
    }

    // What the actual fuck sqlite. Need to enable foreign keys or else we won't get
    // propagation of deletes and what not via cascade.
    SqlStatement stmt(_db, "PRAGMA foreign_keys = ON;");
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to enable foreign keys.");
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

    // First *delete the match if it exists*.
    // Assume that the incoming match is always a suitable replacement.
    // The VOD path is not included in this so it's up to the caller to make sure
    // the VOD path is replaced. Shitty design? Yes. :(
    SqlStatement stmt(_db, "DELETE FROM valorant_matches WHERE id = ?");
    stmt.bindParameter(1, match->matchId());
    tx.exec(stmt);
    if (tx.fail()) {
        THROW_ERROR("Failed to delete valorant match: " << tx.errMsg());
    }

    insertValorantMatchSql(tx, match);

    if (tx.fail()) {
        THROW_ERROR("Failed to store valorant match: " << tx.errMsg());
    }
}

service::valorant::ValorantMatchPtr DatabaseApi::getValorantMatch(const std::string& matchId) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT 
            id,
            gameMode,
            map,
            is_ranked,
            provisioning_flow_id,
            game_version,
            server_start_time_utc,
            start_time_utc,
            end_time_utc,
            raw_api_data
        FROM valorant_matches WHERE id = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (!stmt.hasNext()) {
        return nullptr;
    }

    if (stmt.fail()) {
        THROW_ERROR("Failed to get stored match: " << stmt.errMsg());
    }

    shared::TimePoint startTime = shared::TimePoint::max();
    if (!stmt.isColumnNull(7)) {
        startTime = stmt.getTimeColumnFromString(7, "%F %H:%M:%S.000");
    }

    shared::TimePoint endTime = shared::TimePoint::max();
    if (!stmt.isColumnNull(8)) {
        endTime = stmt.getTimeColumnFromString(8, "%F %H:%M:%S.000");
    }

    auto match = std::make_unique<service::valorant::ValorantMatch>(
        startTime,
        shared::valorant::mapIdToValorantMap(stmt.getColumn<std::string>(2)),
        matchId
    );
    match->finishMatch(endTime);

    service::valorant::ValorantMatchDetails details;
    details.setGameMode(stmt.getColumn<std::string>(1));
    details.setMap(stmt.getColumn<std::string>(2));
    details.setIsRanked(stmt.getColumn<int>(3));
    details.setProvisioningFlowID(stmt.getColumn<std::string>(4));
    details.setGameVersion(stmt.getColumn<std::string>(5));
    details.setStartTime(stmt.getColumn<long long>(6));
    details.setMatchId(stmt.getColumn<std::string>(0));

    if (!stmt.isColumnNull(9)) {
        details.setRawApiData(stmt.getColumn<std::string>(9));
    }

    {
        auto teams = getValorantMatchTeams(matchId, details);
        details.setTeams(teams);
    }

    {
        auto players = getValorantMatchPlayers(matchId, details);
        details.setPlayers(players);
    }

    details.setRounds(getValorantMatchRounds(matchId, details));
    details.setKills(getValorantMatchKills(matchId, details));
    details.setDamage(getValorantMatchDamage(matchId, details));

    match->setDetails(std::move(details));
    return match;
}


std::vector<service::valorant::ValorantMatchPlayerPtr> DatabaseApi::getValorantMatchPlayers(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const {
    std::vector<service::valorant::ValorantMatchPlayerPtr> players;

    std::ostringstream sql;
    sql << R"|(
        SELECT 
            team_id,
            puuid,
            agent_id,
            competitive_tier,
            total_combat_score,
            rounds_played,
            kills,
            deaths,
            assists
        FROM valorant_match_players WHERE match_id = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match players: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        auto player = std::make_unique<service::valorant::ValorantMatchPlayer>();
        player->setTeam(details.teams().at(stmt.getColumn<std::string>(0)).get());
        player->setPuuid(stmt.getColumn<std::string>(1));
        player->setAgentId(stmt.getColumn<std::string>(2));
        player->setCompetitiveTier(stmt.getColumn<int>(3));
        player->setStatCombatScore(stmt.getColumn<int>(4));
        player->setStatRoundsPlayed(stmt.getColumn<int>(5));
        player->setStatKills(stmt.getColumn<int>(6));
        player->setStatDeaths(stmt.getColumn<int>(7));
        player->setStatAssists(stmt.getColumn<int>(8));
        players.emplace_back(std::move(player));
        stmt.next();
    }

    return players;
}

std::vector<service::valorant::ValorantMatchTeamPtr> DatabaseApi::getValorantMatchTeams(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const {
    std::vector<service::valorant::ValorantMatchTeamPtr> teams;
    std::ostringstream sql;
    sql << R"|(
        SELECT 
            team_id,
            won,
            rounds_won,
            rounds_played
        FROM valorant_match_teams WHERE match_id = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match teams: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        auto team = std::make_unique<service::valorant::ValorantMatchTeam>();
        team->setTeamId(stmt.getColumn<std::string>(0));
        team->setWon(stmt.getColumn<int>(1));
        team->setRoundsWon(stmt.getColumn<int>(2));
        team->setRoundsPlayed(stmt.getColumn<int>(3));
        teams.emplace_back(std::move(team));
        stmt.next();
    }
    return teams;
}

std::vector<service::valorant::ValorantMatchRoundPtr> DatabaseApi::getValorantMatchRounds(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const {
    std::vector<service::valorant::ValorantMatchRoundPtr> rounds;

    std::ostringstream sql;
    sql << R"|(
        SELECT 
            round_num,
            plant_round_time,
            planter_puuid,
            defuse_round_time,
            defuser_puuid,
            team_round_winner,
            round_buy_time_utc,
            round_play_time_utc
        FROM valorant_match_rounds WHERE match_id = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match rounds: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        auto round = std::make_unique<service::valorant::ValorantMatchRound>();
        round->setRoundNum(stmt.getColumn<int>(0));
        
        if (!stmt.isColumnNull(1) && !stmt.isColumnNull(2)) {
            round->setPlantTime(stmt.getColumn<int>(1));
            round->setPlanter(details.players().at(stmt.getColumn<std::string>(2)).get());
        }

        if (!stmt.isColumnNull(3) && !stmt.isColumnNull(4)) {
            round->setDefuseTime(stmt.getColumn<int>(3));
            round->setDefuser(details.players().at(stmt.getColumn<std::string>(4)).get());
        }

        round->setTeamWinner(stmt.getColumn<std::string>(5));

        if (!stmt.isColumnNull(6)) {
            round->setStartBuyTime(stmt.getTimeColumnFromString(6, "%F %H:%M:%S.000"));
        }

        if (!stmt.isColumnNull(7)) {
            round->setStartBuyTime(stmt.getTimeColumnFromString(7, "%F %H:%M:%S.000"));
        }

        round->setLoadouts(getValorantMatchRoundLoadouts(matchId, round->roundNum(), details));
        round->setCombatScores(getValorantMatchRoundStats(matchId, round->roundNum()));

        rounds.emplace_back(std::move(round));
        stmt.next();
    }

    return rounds;
}

std::vector<service::valorant::ValorantMatchLoadoutPtr> DatabaseApi::getValorantMatchRoundLoadouts(const std::string& matchId, int roundNum, const service::valorant::ValorantMatchDetails& details) const {
    std::vector<service::valorant::ValorantMatchLoadoutPtr> loadouts;

    std::ostringstream sql;
    sql << R"|(
        SELECT 
            puuid,
            loadout_value,
            remaining_money,
            spent_money,
            weapon,
            armor
        FROM valorant_match_round_player_loadout WHERE match_id = ? AND round_num = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.bindParameter(2, roundNum);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match round loadouts: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        auto loadout = std::make_unique<service::valorant::ValorantMatchLoadout>();
        loadout->setOwner(details.players().at(stmt.getColumn<std::string>(0)).get());
        loadout->setLoadoutValue(stmt.getColumn<int>(1));
        loadout->setRemainingMoney(stmt.getColumn<int>(2));
        loadout->setSpentMoney(stmt.getColumn<int>(3));
        loadout->setWeapon(stmt.getColumn<std::string>(4));
        loadout->setArmor(stmt.getColumn<std::string>(5));

        loadouts.emplace_back(std::move(loadout));
        stmt.next();
    }
    return loadouts;
}

std::unordered_map<std::string, int> DatabaseApi::getValorantMatchRoundStats(const std::string& matchId, int roundNum) const {
    std::unordered_map<std::string, int> scores;

    std::ostringstream sql;
    sql << R"|(
        SELECT 
            puuid,
            combat_score
        FROM valorant_match_round_player_stats WHERE match_id = ? AND round_num = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.bindParameter(2, roundNum);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match round stats: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        scores[stmt.getColumn<std::string>(0)] = stmt.getColumn<int>(1);
        stmt.next();
    }
    return scores;
}

std::vector<service::valorant::ValorantMatchKillPtr> DatabaseApi::getValorantMatchKills(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const {
    std::vector<service::valorant::ValorantMatchKillPtr> kills;

    std::ostringstream sql;
    sql << R"|(
        SELECT 
            round_num,
            killer_puuid,
            victim_puuid,
            round_time,
            damage_type,
            damage_item,
            is_secondary_fire
        FROM valorant_match_kill WHERE match_id = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match kills: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        auto kill = std::make_unique<service::valorant::ValorantMatchKill>();
        kill->setRoundTime(stmt.getColumn<int>(3));
        kill->setRound(stmt.getColumn<int>(0));
        kill->setDamageType(stmt.getColumn<std::string>(4));
        kill->setDamageItem(stmt.getColumn<std::string>(5));
        kill->setKillSecondaryFire(stmt.getColumn<int>(6));
        kill->setKiller(details.players().at(stmt.getColumn<std::string>(1)).get());
        kill->setVictim(details.players().at(stmt.getColumn<std::string>(2)).get());

        kills.emplace_back(std::move(kill));
        stmt.next();
    }
    return kills;
}

std::vector<service::valorant::ValorantMatchDamagePtr> DatabaseApi::getValorantMatchDamage(const std::string& matchId, const service::valorant::ValorantMatchDetails& details) const {
    std::vector<service::valorant::ValorantMatchDamagePtr> damage;

    std::ostringstream sql;
    sql << R"|(
        SELECT 
            round_num,
            instigator_puuid,
            receiver_puuid,
            damage,
            legshots,
            bodyshots,
            headshots
        FROM valorant_match_damage WHERE match_id = ?)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get match damage: " << stmt.errMsg());
    }

    while(stmt.hasNext()) {
        auto dmg = std::make_unique<service::valorant::ValorantMatchDamage>();
        dmg->setRoundNum(stmt.getColumn<int>(0));
        dmg->setDamage(stmt.getColumn<int>(3));
        dmg->setLegshots(stmt.getColumn<int>(4));
        dmg->setBodyshots(stmt.getColumn<int>(5));
        dmg->setHeadshots(stmt.getColumn<int>(6));
        dmg->setInstigator(details.players().at(stmt.getColumn<std::string>(1)).get());
        dmg->setReceiver(details.players().at(stmt.getColumn<std::string>(2)).get());

        damage.emplace_back(std::move(dmg));
        stmt.next();
    }
    return damage;
}

size_t DatabaseApi::totalValorantMatchesForPuuid(const std::string& puuid, bool withApiData) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT COUNT(vm.id)
        FROM valorant_matches AS vm
        INNER JOIN valorant_match_players AS vmp
            ON vmp.match_id = vm.id
        WHERE vmp.puuid = ?)|";

    if (withApiData) {
        sql << " AND vm.raw_api_data IS NOT NULL";
    }

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, puuid);
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
        WHERE vm.id = ? AND vm.raw_api_data IS NOT NULL)|";

    SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, matchId);
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

std::string DatabaseApi::getVodFilenameForValorantMatch(const std::string& matchId) const {
    SqlStatement stmt(_db, "SELECT video_path FROM valorant_match_videos WHERE match_id = ?");
    stmt.bindParameter(1, matchId);
    stmt.next();

    if (!stmt.hasNext()) {
        return "";
    }

    if (stmt.fail()) {
        THROW_ERROR("Failed to store valorant match vod: " << stmt.errMsg());
    }

    return stmt.getColumn<std::string>(0);
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
    stmt.bindParameter(5, shared::timeToStr(task.createDate));
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