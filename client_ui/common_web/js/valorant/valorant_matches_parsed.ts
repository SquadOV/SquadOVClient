import { 
    ValorantMatchDetails,
    ValorantMatchTeam,
    ValorantMatchPlayer,
    ValorantMatchRoundResult,
    ValorantMatchKill,
    ValorantMatchEconomy,
    ValorantMatchDamage,
    ValorantMatchPlayerRoundStats,
    getIsDeathmatch
} from '@client/js/valorant/valorant_matches'
import { getValorantContent } from '@client/js/valorant/valorant_content'

export class ValorantMatchDamageWrapper {
    _d : ValorantMatchDamage

    instigator: ValorantMatchPlayerWrapper
    receiver: ValorantMatchPlayerWrapper
    roundNum: number

    constructor(d : ValorantMatchDamage, instigator : ValorantMatchPlayerWrapper, receiver : ValorantMatchPlayerWrapper, roundNum: number) {
        this._d = d
        this.instigator = instigator
        this.receiver = receiver
        this.roundNum = roundNum
    }
}

export class ValorantMatchKillWrapper {
    _k : ValorantMatchKill

    killer: ValorantMatchPlayerWrapper | null
    victim: ValorantMatchPlayerWrapper
    roundNum: number

    constructor(k : ValorantMatchKill, killer : ValorantMatchPlayerWrapper | null, victim : ValorantMatchPlayerWrapper, roundNum: number) {
        this._k = k
        this.killer = killer
        this.victim = victim
        this.roundNum = roundNum
    }

    get isSelfTeamKill() : boolean {
        return (this.killer?._p.puuid == this.victim._p.puuid) ||
            (this.killer?._p.teamId == this.victim._p.teamId)
    }
}

export class ValorantMatchRoundWrapper {
    _r : ValorantMatchRoundResult
    kills : ValorantMatchKillWrapper[]
    damage : ValorantMatchDamageWrapper[]
    playerToLoadout : { [ puuid : string ] : ValorantMatchEconomy } 
    playerToStats : { [ puuid : string ]  : ValorantMatchPlayerRoundStats }

    constructor(r : ValorantMatchRoundResult) {
        this._r = r
        this.kills = []
        this.damage = []
        this.playerToLoadout = {}
        this.playerToStats = {}

        for (let stat of this._r.playerStats) {
            this.playerToStats[stat.puuid] = stat
            this.playerToLoadout[stat.puuid] = stat.economy
        }
    }

    getKillsForPlayer(puuid : string) : number {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return ele.killer?._p.puuid == puuid
        }).length
    }

    getDeathsForPlayer(puuid : string) : number {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return ele.victim._p.puuid == puuid
        }).length
    }

    getAssistsForPlayer(puuid : string) : number {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return (ele._k.assistants.findIndex((ele: string) => ele === puuid) != -1)
        }).length
    }

    getCombatScoreForPlayer(puuid : string) : number {
        return this.playerToStats[puuid].score
    }

    getDamageDoneByPlayer(puuid : string) : number {
        return this.damage.filter((ele : ValorantMatchDamageWrapper) => {
            return (ele.instigator._p.puuid == puuid)
        }).reduce((acc, ele) => {
            return acc + ele._d.damage
        }, 0)
    }

    getLoadoutForPlayer(puuid : string) : ValorantMatchEconomy {
        return this.playerToLoadout[puuid]
    }

    get sortedKills() : ValorantMatchKillWrapper[] {
        return [...this.kills].sort((a : ValorantMatchKillWrapper, b : ValorantMatchKillWrapper) => {
            return a._k.timeSinceGameStartMillis - b._k.timeSinceGameStartMillis
        })
    }
}

export class ValorantMatchPlayerWrapper {
    _p : ValorantMatchPlayer
    damage : ValorantMatchDamageWrapper[]
    kills : ValorantMatchKillWrapper[]
    deaths : ValorantMatchKillWrapper[]

    constructor(p : ValorantMatchPlayer) {
        this._p = p
        this.damage = []
        this.kills = []
        this.deaths = []
    }

    get kda() : number {
        return (this._p.stats.kills + this._p.stats.assists) / this._p.stats.deaths
    }

    get cspr() : number {
        return (this._p.stats.roundsPlayed == 0) ? 0.0 : this._p.stats.score / this._p.stats.roundsPlayed
    }

    get dpr() : number {
        return (this._p.stats.roundsPlayed == 0) ? 0.0 : this.damage.reduce((acc : number, curr : ValorantMatchDamageWrapper) => {
            return acc + curr._d.damage
        }, 0.0) / this._p.stats.roundsPlayed
    }

    get headshots() : number {
        return this.damage.reduce((acc : number, curr : ValorantMatchDamageWrapper) => {
            return acc + curr._d.headshots
        }, 0)
    }

    get bodyshots() : number {
        return this.damage.reduce((acc : number, curr : ValorantMatchDamageWrapper) => {
            return acc + curr._d.bodyshots
        }, 0)
    }

    get legshots() : number {
        return this.damage.reduce((acc : number, curr : ValorantMatchDamageWrapper) => {
            return acc + curr._d.legshots
        }, 0)
    }

    get totalShots() : number {
        return this.headshots + this.bodyshots + this.legshots
    }

    get hsp() : number {
        return (this.totalShots == 0) ? 0 : this.headshots / this.totalShots
    }

    get perRoundKills() : number[] {
        let killsPerRound : any = {}
        let maxRound = 0
        this.kills.forEach((kill : ValorantMatchKillWrapper) => {
            if (kill.roundNum in killsPerRound) {
                killsPerRound[kill.roundNum] += 1
            } else {
                killsPerRound[kill.roundNum] = 1
            }
            maxRound = Math.max(kill.roundNum)
        })

        let ret = new Array(maxRound + 1)
        for (let i = 0; i < maxRound + 1; i++) {
            if (i in killsPerRound) {
                ret[i] = killsPerRound[i]
            }
        }
        return ret
    }

    get doubleKills() : number {
        return this.perRoundKills.filter((ele : number) => ele == 2).length
    }

    get tripleKills() : number {
        return this.perRoundKills.filter((ele : number) => ele == 3).length
    }

    get quadraKills() : number {
        return this.perRoundKills.filter((ele : number) => ele == 4).length
    }

    get pentaKills() : number {
        return this.perRoundKills.filter((ele : number) => ele == 5).length
    }
}

interface MatchPlayerMap {
    [ puuid : string ] : ValorantMatchPlayerWrapper
}

export class ValorantMatchTeamWrapper {
    _t : ValorantMatchTeam
    players: ValorantMatchPlayerWrapper[]
    puuidToPlayer: Map<string, ValorantMatchPlayerWrapper>

    constructor(t : ValorantMatchTeam) {
        this._t = t
        this.players = []
        this.puuidToPlayer = new Map()
    }

    hasPlayer(p: string): boolean {
        return this.puuidToPlayer.has(p)
    }

    addPlayer(p : ValorantMatchPlayerWrapper) {
        this.players.push(p)
        this.puuidToPlayer.set(p._p.puuid, p)
    }

    getPlayersDescendingCS() : ValorantMatchPlayerWrapper[] {
        return [...this.players].sort((a : ValorantMatchPlayerWrapper, b : ValorantMatchPlayerWrapper) => {
            return b._p.stats.score - a._p.stats.score
        })
    }
}

interface MatchTeamMap {
    [ team : string ] : ValorantMatchTeamWrapper
}

export class ValorantMatchDetailsWrapper {
    _details: ValorantMatchDetails
    _players : MatchPlayerMap
    _teams : MatchTeamMap
    _rounds : ValorantMatchRoundWrapper[]
    _kills : ValorantMatchKillWrapper[]
    _damage : ValorantMatchDamageWrapper[]

    constructor(details : ValorantMatchDetails) {
        this._details = details

        this._teams = {}
        for (let t of this._details.data.teams) {
            this._teams[t.teamId] = new ValorantMatchTeamWrapper(t)
        }

        this._players = {}
        for (let p of this._details.data.players) {
            let np = new ValorantMatchPlayerWrapper(p)
            this._teams[p.teamId].addPlayer(np)
            this._players[p.puuid] = np
        }

        this._kills = []
        this._damage = []
        this._rounds = []

        this._details.data.roundResults.forEach((ele: ValorantMatchRoundResult) => {
            let rnd = new ValorantMatchRoundWrapper(ele)
            ele.playerStats.forEach((st: ValorantMatchPlayerRoundStats) => {
                st.kills.forEach((kill: ValorantMatchKill) => {
                    let killer = !!kill.killer ? this._players[kill.killer] : null
                    let victim = this._players[kill.victim]

                    let k = new ValorantMatchKillWrapper(
                        kill,
                        killer,
                        victim,
                        ele.roundNum
                    )
                    this._kills.push(k)
                    rnd.kills.push(k)

                    if (!!killer) {
                        killer.kills.push(k)
                    }
                    
                    victim.deaths.push(k)
                })

                st.damage.forEach((damage: ValorantMatchDamage) => {
                    let instigator = this._players[st.puuid]
                    let receiver = this._players[damage.receiver]

                    let d = new ValorantMatchDamageWrapper(
                        damage,
                        instigator,
                        receiver,
                        ele.roundNum
                    )
                    this._damage.push(d)
                    rnd.damage.push(d)
                    instigator.damage.push(d)
                })
            })
            this._rounds.push(rnd)
        })
    }

    get isDeathmatch(): boolean {
        return getIsDeathmatch(this._details.data.matchInfo.gameMode)
    }

    getDamageDoneByPlayer(puuid : string) : number {
        return this._damage.filter((ele : ValorantMatchDamageWrapper) => {
            return (ele.instigator._p.puuid == puuid)
        }).reduce((acc, ele) => {
            return acc + ele._d.damage
        }, 0)
    }

    getPlayersDescendingCS() : ValorantMatchPlayerWrapper[] {
        return this._details.data.players.map((ele : ValorantMatchPlayer) => {
            return this._players[ele.puuid]
        }).sort((a : ValorantMatchPlayerWrapper, b : ValorantMatchPlayerWrapper) => {
            return b._p.stats.score - a._p.stats.score
        })
    }

    getMatchStartTime() : Date | null {
        return this._details.data.matchInfo.serverStartTimeUtc
    }

    getPlayer(puuid : string) : ValorantMatchPlayerWrapper {
        return this._players[puuid]
    }

    getPlayerAgentId(puuid : string) : string {
        return this.getPlayer(puuid)._p.characterId
    }

    getPlayerAgentName(puuid : string) : string | null {
        let cnt = getValorantContent(null)
        return cnt.agentIdToName(this.getPlayerAgentId(puuid))
    }

    getPlayerTeam(puuid : string) : ValorantMatchTeamWrapper {
        let p = this._players[puuid]
        return this._teams[p._p.teamId]
    }

    getOpposingPlayerTeam(puuid : string) : ValorantMatchTeamWrapper {
        let p = this._players[puuid]
        let team = p._p.teamId

        if (team == 'Blue') {
            return this.getTeam('Red')
        } else {
            return this.getTeam('Blue')
        }
    }

    getTeam(team : string) : ValorantMatchTeamWrapper {
        return this._teams[team]
    }

    getNumRounds() : number {
        return this._rounds.length
    }

    getRound(rnd : number) : ValorantMatchRoundWrapper | null {
        if (rnd < 0 || rnd >= this._rounds.length) {
            return null
        }
        return this._rounds[rnd]
    }

    firstBloods(puuid : string) : number {
        return this._rounds.reduce((acc : number, ele : ValorantMatchRoundWrapper) => {
            if (ele.kills.length == 0) {
                return acc
            }

            if (ele.sortedKills[0].killer?._p.puuid == puuid) {
                acc += 1
            }

            return acc
        }, 0)
    }

    firstToDie(puuid : string) : number {
        return this._rounds.reduce((acc : number, ele : ValorantMatchRoundWrapper) => {
            if (ele.kills.length == 0) {
                return acc
            }

            if (ele.sortedKills[0].victim._p.puuid == puuid) {
                acc += 1
            }

            return acc
        }, 0)
    }
}