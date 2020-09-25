const { V4MAPPED } = require('dns')
const express = require('express')

class ValorantApiServer {
    constructor(db) {
        this.db = db
    }

    createRouter() {
        const valorantRouter = express.Router()
        valorantRouter.get('/accounts', this.listValorantAccounts.bind(this))
        valorantRouter.get('/accounts/:puuid', this.getValorantAccount.bind(this))

        valorantRouter.get('/matches/:puuid', this.listValorantMatches.bind(this))

        valorantRouter.get('/stats/summary/:puuid', this.getPlayerStatsSummary.bind(this))
        return valorantRouter
    }
    
    listValorantAccounts(req, res) { 
        this.db.all(`
        SELECT *
        FROM valorant_accounts
        `, [], (err, rows) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(rows)
        })
    }

    getValorantAccount(req, res) {
        this.db.get(`
            SELECT *
            FROM valorant_accounts
            WHERE puuid = ?
        `, [req.params['puuid']], (err, row) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(row)            
        })
    }

    listValorantMatches(req, res) {
        this.db.all(`
            SELECT
                vm.id AS "matchId",
                vm.server_start_time_utc AS "matchTime",
                vm.map AS "map",
                vm.is_ranked AS "isRanked",
                vm.gameMode AS "gameMode",
                vm.provisioning_flow_id AS "provisioningFlowId",
                vmp.agent_id AS "agentId",
                vmt.won AS "won",
                vm.game_version AS "patchId",
                vmv.video_path IS NOT NULL AS "hasVod",
                (
                    SELECT COUNT(puuid) + 1
                    FROM valorant_match_players
                    WHERE match_id = vm.id
                        AND total_combat_score > vmp.total_combat_score
                ) AS "csRank",
                vmt.rounds_won AS "myTeamScore",
                MAX(vmto.rounds_won) AS "otherTeamScore",
                vvpms.rank AS "stats.rank",
                vvpms.kills AS "stats.kills",
                vvpms.deaths AS "stats.deaths",
                vvpms.assists AS "stats.assists",
                vvpms.rounds AS "stats.rounds",
                vvpms.totalCombatScore AS "stats.totalCombatScore",
                vvpms.totalDamage AS "stats.totalDamage",
                vvpms.headshots AS "stats.headshots",
                vvpms.bodyshots AS "stats.bodyshots",
                vvpms.legshots AS "stats.legshots",
                vvpms.won AS "stats.wins"
            FROM valorant_matches AS vm
            INNER JOIN valorant_match_players AS vmp
                ON vmp.match_id = vm.id
            INNER JOIN valorant_match_teams AS vmt
                ON vmt.team_id = vmp.team_id AND vmt.match_id = vm.id
            INNER JOIN valorant_match_teams AS vmto
                ON vmto.team_id != vmp.team_id AND vmto.match_id = vm.id
            INNER JOIN view_valorant_player_match_stats AS vvpms
                ON vvpms.puuid = vmp.puuid AND vvpms.matchId = vmp.match_id
            LEFT JOIN valorant_match_videos AS vmv
                ON vmv.match_id = vm.id
            WHERE vmp.puuid = ?
            GROUP BY vm.id
            ORDER BY vm.server_start_time_utc DESC
        `, [req.params['puuid']], (err, rows) => {
            if (!!err) {
                res.status(500).json({ 'error': err })
            } else {
                res.json(rows.map((ele) => {
                    return {
                        matchId: ele.matchId,
                        matchTime: new Date(ele.matchTime),
                        map: ele.map,
                        provisioningFlowId: ele.provisioningFlowId,
                        gameMode: ele.gameMode,
                        patchId: ele.patchId,
                        hasVod: ele.hasVod,
                        myTeamScore: ele.myTeamScore,
                        otherTeamScore: ele.otherTeamScore,
                        win: ele.won,
                        agentId: ele.agentId,
                        stats: {
                            rank : ele['stats.rank'],
                            kills : ele['stats.kills'],
                            deaths : ele['stats.deaths'],
                            assists : ele['stats.assists'],
                            rounds : ele['stats.rounds'],
                            totalCombatScore : ele['stats.totalCombatScore'],
                            totalDamage : ele['stats.totalDamage'],
                            headshots: ele['stats.headshots'],
                            bodyshots : ele['stats.bodyshots'],
                            legshots : ele['stats.legshots'],
                            wins: ele['stats.wins'],
                            games: 1,
                        },
                        csRank: ele.csRank,
                    }
                }))
            }        
        })        
    }

    getPlayerStatsSummary(req, res) {
        this.db.get(`
            SELECT
                COALESCE((
                    SELECT vmp.competitive_tier
                    FROM valorant_matches AS vm
                    INNER JOIN valorant_match_players AS vmp
                        ON vmp.match_id = vm.id
                    WHERE vmp.puuid = ?
                        AND vm.is_ranked IS TRUE
                    ORDER BY vm.server_start_time_utc DESC
                    LIMIT 1
                ), 0) AS "rank",
                SUM(vvpms.kills) AS "kills",
                SUM(vvpms.deaths) AS "deaths",
                SUM(vvpms.assists) AS "assists",
                SUM(vvpms.rounds) AS "rounds",
                SUM(vvpms.totalCombatScore) AS "totalCombatScore",
                SUM(vvpms.totalDamage) AS "totalDamage",
                SUM(vvpms.headshots) AS "headshots",
                SUM(vvpms.bodyshots) AS "bodyshots",
                SUM(vvpms.legshots) AS "legshots",
                SUM(vvpms.won) AS "wins",
                COUNT(vvpms.matchId) AS "games"
            FROM view_valorant_player_match_stats AS vvpms
            WHERE vvpms.puuid = ?
            GROUP BY vvpms.puuid
        `, [req.params['puuid'], req.params['puuid']], (err, row) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(row)            
        })
    }
}

module.exports.ValorantApiServer = ValorantApiServer