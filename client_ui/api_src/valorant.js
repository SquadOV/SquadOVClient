const express = require('express')
const { RiotRsoTokenRetriever }  = require('./riot/rso.js')
const { encryptPassword, decryptPassword } = require('./password.js')

class ValorantApiServer {
    constructor(db) {
        this.db = db
    }

    createRouter() {
        const valorantRouter = express.Router()
        valorantRouter.use(express.json())
        valorantRouter.get('/accounts', this.listValorantAccounts.bind(this))
        valorantRouter.post('/accounts', this.createValorantAccount.bind(this))
        valorantRouter.get('/accounts/:puuid', this.getValorantAccount.bind(this))
        valorantRouter.get('/accounts/:puuid/rso', this.getValorantAccountTokens.bind(this))
        valorantRouter.put('/accounts/:puuid', this.editValorantAccount.bind(this))

        valorantRouter.get('/accounts/:puuid/matches', this.listValorantMatches.bind(this))
        valorantRouter.get('/matches/:matchId', this.getValorantMatchDetails.bind(this))

        valorantRouter.get('/stats/summary/:puuid', this.getPlayerStatsSummary.bind(this))
        return valorantRouter
    }
    
    listValorantAccounts(req, res) { 
        this.db.all(`
        SELECT
            puuid,
            username,
            tag,
            login,
            encrypted_password AS "encryptedPassword"
        FROM valorant_accounts
        `, [], (err, rows) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(rows)
        })
    }

    getValorantAccountTokens(req, res) {
        // Get the login information stored in the database.
        this.db.get(`
            SELECT login, encrypted_password AS "encryptedPassword"
            FROM valorant_accounts
            WHERE puuid = ?
        `, [req.params['puuid']], (err, row) => {
            if (!!err) {
                res.status(500).json({'error': err})
                return
            }

            new RiotRsoTokenRetriever(row.login, decryptPassword(row.encryptedPassword)).obtain(true).then(({rso, user}) => {
                res.json(rso)
            }).catch((err) => {
                res.status(500).json({'error': err})
            })
        })
    }

    getValorantAccount(req, res) {
        this.db.get(`
            SELECT
                puuid,
                username,
                tag,
                login,
                encrypted_password AS "encryptedPassword"
            FROM valorant_accounts
            WHERE puuid = ?
        `, [req.params['puuid']], (err, row) => {
            if (!!err) res.status(500).json({ 'error': err })
            else res.json(row)            
        })
    }

    createValorantAccount(req, res) {
        // Verify that the username/password combination works by
        // attempting to obtain an RSO token.
        new RiotRsoTokenRetriever(req.body.login, req.body.password).obtain().then(({rso, user}) => {
            this.db.serialize(() => {
                this.db.run('BEGIN EXCLUSIVE TRANSACTION;')
                this.db.run(`
                    INSERT INTO valorant_accounts (
                        puuid,
                        username,
                        tag,
                        login,
                        encrypted_password
                    )
                    VALUES (
                        ?,
                        ?,
                        ?,
                        ?,
                        ?
                    )
                `, [
                    user.puuid,
                    user.username,
                    user.tag,
                    req.body.login,
                    encryptPassword(req.body.password)
                ])
                this.db.exec('COMMIT TRANSACTION;')

                req.params['puuid'] = user.puuid
                this.getValorantAccount(req, res)
            })
        }).catch((err) => {
            res.status(500).json({'error': err})
        })
    }

    editValorantAccount(req, res) {
        // Verify that the username/password combination works by
        // attempting to obtain an RSO token AND make sure that the
        // puuid matches the specified PUUID.
        new RiotRsoTokenRetriever(req.body.login, req.body.password).obtain().then(({rso, user}) => {
            const refPuuid = req.params.puuid
            if (refPuuid !== user.puuid) {
                res.status(500).json({'error': `PUUID mismatch: ${refPuuid} vs ${user.puuid}`})
                return
            }

            this.db.serialize(() => {
                this.db.run('BEGIN EXCLUSIVE TRANSACTION;')
                this.db.run(`
                    UPDATE valorant_accounts
                    SET login = ?,
                        encrypted_password = ?
                    WHERE puuid = ?
                `, [
                    req.body.login,
                    encryptPassword(req.body.password),
                    user.puuid,
                ])
                this.db.exec('COMMIT TRANSACTION;')
                this.getValorantAccount(req, res)
            })
        }).catch((err) => {
            res.status(500).json({'error': err})
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
                        matchTime: new Date(ele.matchTime * 1000),
                        map: ele.map,
                        isRanked: ele.isRanked,
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

    getValorantMatchDetails(req, res) {
        const matchId = req.params['matchId']
        
        this.db.serialize(() => {
            let match = {
                players: [],
                teams: [],
                rounds: [],
                kills: [],
                damage: [],                
            }
            let errs = []
        
            this.db.get(`
                SELECT
                    vm.id AS "matchId",
                    vm.server_start_time_utc AS "matchTime",
                    vm.start_time_utc AS "ovStartTime",
                    vm.end_time_utc AS "ovEndTime",
                    vm.map AS "map",
                    vm.provisioning_flow_id AS "provisioningFlowId",
                    vm.gameMode AS "gameMode",
                    vm.game_version AS "patchId",
                    vm.is_ranked AS "isRanked",
                    vmv.video_path AS "vodPath"
                FROM valorant_matches AS vm
                LEFT JOIN valorant_match_videos AS vmv
                    ON vmv.match_id = vm.id
                WHERE vm.id = ?
            `, [matchId], (e, row) => {
                if (!!e) {
                    errs.push(e)
                    return
                }

                match = {
                    ...match,
                    ...row,
                }

                match.matchTime = new Date(match.matchTime * 1000)

                if (!!match.ovStartTime) {
                    match.ovStartTime = new Date(match.ovStartTime)
                }

                if (!!match.ovEndTime) {
                    match.ovEndTime = new Date(match.ovEndTime)
                }
            })

            this.db.parallelize(() => {
                this.db.all(`
                    SELECT
                        vmp.puuid AS "puuid",
                        vmp.team_id AS "teamId",
                        vmp.agent_id AS "agentId",
                        vmp.competitive_tier AS "competitiveTier",
                        vmp.total_combat_score AS "totalCombatScore",
                        vmp.rounds_played AS "roundsPlayed",
                        vmp.kills AS "kills",
                        vmp.deaths AS "deaths",
                        vmp.assists AS "assists"
                    FROM valorant_match_players AS vmp
                    WHERE vmp.match_id = ?
                `, [matchId], (e, rows) => {
                    if (!!e) {
                        errs.push(e)
                        return
                    }

                    match.players = rows
                })

                this.db.all(`
                    SELECT
                        team_id AS "teamId",
                        won AS "won",
                        rounds_won AS "roundsWon",
                        rounds_played AS "roundsPlayed"
                    FROM valorant_match_teams
                    WHERE match_id = ?
                `, [matchId], (e, rows) => {
                    if (!!e) {
                        errs.push(e)
                        return
                    }

                    match.teams = rows
                })

                this.db.all(`
                    SELECT
                        round_buy_time_utc AS "startBuyTime",
                        round_play_time_utc AS "startPlayTime",
                        round_num AS "roundNum",
                        plant_round_time AS "plantRoundTime",
                        planter_puuid AS "planter",
                        defuse_round_time AS "defuseRoundTime",
                        defuser_puuid AS "defuser",
                        team_round_winner AS "roundWinner"
                    FROM valorant_match_rounds
                    WHERE match_id = ?
                    ORDER BY round_num ASC
                `, [matchId], (e, rows) => {
                    if (!!e) {
                        errs.push(e)
                        return
                    }

                    match.rounds = rows

                    match.rounds.forEach((rnd) => {
                        this.db.all(`
                            SELECT
                                round_num AS "roundNum",
                                puuid AS "puuid",
                                loadout_value AS "loadoutValue",
                                remaining_money AS "remainingMoney",
                                spent_money AS "spentMoney",
                                weapon AS "weapon",
                                armor AS "armor"
                            FROM valorant_match_round_player_loadout
                            WHERE match_id = ? AND round_num = ?
                        `, [matchId, rnd.roundNum], (e, rows) => {
                            if (!!e) {
                                errs.push(e)
                                return
                            }
                            rnd.loadouts = rows
                        })

                        this.db.all(`
                            SELECT
                                round_num AS "roundNum",
                                puuid AS "puuid",
                                combat_score AS "combatScore"
                            FROM valorant_match_round_player_stats
                            WHERE match_id = ? AND round_num = ?
                        `, [matchId, rnd.roundNum], (e, rows) => {
                            if (!!e) {
                                errs.push(e)
                                return
                            }
                            rnd.roundStats = rows
                        })
                    })
                })

                this.db.all(`
                    SELECT
                        round_num AS "roundNum",
                        round_time AS "roundTime",
                        damage_type AS "damageType",
                        damage_item AS "damageItem",
                        is_secondary_fire AS "secondaryFire",
                        killer_puuid AS "killer",
                        victim_puuid AS "victim"
                    FROM valorant_match_kill
                    WHERE match_id = ?
                `, [matchId], (e, rows) => {
                    if (!!e) {
                        errs.push(e)
                        return
                    }

                    match.kills = rows
                })

                this.db.all(`
                    SELECT
                        round_num AS "roundNum",
                        damage AS "damage",
                        headshots AS "headshots",
                        bodyshots AS "bodyshots",
                        legshots AS "legshots",
                        instigator_puuid AS "instigator",
                        receiver_puuid AS "receiver"
                    FROM valorant_match_damage
                    WHERE match_id = ?
                `, [matchId], (e, rows) => {
                    if (!!e) {
                        errs.push(e)
                        return
                    }

                    match.damage = rows
                })
            })

            // I think this is the only way to figure out when everything ends.
            this.db.exec(`SELECT 1`, () => {
                if (errs.length > 0) res.status(500).json({ 'errors' : errs })
                else res.json(match)
            })
        })
    }
}

module.exports.ValorantApiServer = ValorantApiServer