import { 
    ValorantMatchDetails,
    ValorantMatchTeam,
    ValorantMatchPlayer,
    ValorantMatchRound,
    ValorantMatchKill,
    ValorantMatchLoadout,
    ValorantMatchDamage,
    ValorantMatchPlayerRoundStat
} from '@client/js/valorant/valorant_matches'
import { getValorantContent } from '@client/js/valorant/valorant_content'

export class ValorantMatchDamageWrapper {
    _d : ValorantMatchDamage

    instigator: ValorantMatchPlayerWrapper
    receiver: ValorantMatchPlayerWrapper

    constructor(d : ValorantMatchDamage, instigator : ValorantMatchPlayerWrapper, receiver : ValorantMatchPlayerWrapper) {
        this._d = d
        this.instigator = instigator
        this.receiver = receiver
    }
}

export class ValorantMatchKillWrapper {
    _k : ValorantMatchKill

    killer: ValorantMatchPlayerWrapper
    victim: ValorantMatchPlayerWrapper

    constructor(k : ValorantMatchKill, killer : ValorantMatchPlayerWrapper, victim : ValorantMatchPlayerWrapper) {
        this._k = k
        this.killer = killer
        this.victim = victim
    }

    get isSelfTeamKill() : boolean {
        return (this.killer._p.puuid == this.victim._p.puuid) ||
            (this.killer._p.teamId == this.victim._p.teamId)
    }
}

export class ValorantMatchRoundWrapper {
    _r : ValorantMatchRound
    kills : ValorantMatchKillWrapper[]
    damage : ValorantMatchDamageWrapper[]
    playerToLoadout : { [ puuid : string ] : ValorantMatchLoadout } 
    playerToStats : { [ puuid : string ]  : ValorantMatchPlayerRoundStat }

    constructor(r : ValorantMatchRound) {
        this._r = r
        this.kills = []
        this.damage = []
        this.playerToLoadout = {}
        this.playerToStats = {}

        for (let stat of this._r.roundStats) {
            this.playerToStats[stat.puuid] = stat
        }

        for (let lo of this._r.loadouts) {
            this.playerToLoadout[lo.puuid] = lo
        }
    }

    getKillsForPlayer(puuid : string) : number {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return ele.killer._p.puuid == puuid
        }).length
    }

    getDeathsForPlayer(puuid : string) : number {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return ele.victim._p.puuid == puuid
        }).length
    }

    getAssistsForPlayer(puuid : string) : number {
        // Intersection of people who died and people who we did >= 50 damage to.
        // TODO: Figure out what happens for team assists (i.e. if we did 50 damage to a teammate).
        // TODO: Probably can make this faster using maps and what not but I don't think the
        // number of kills/damage per round is enough to matter.
        return this.damage.filter((ele : ValorantMatchDamageWrapper) => {
            return (ele.instigator._p.puuid == puuid)
                && (ele._d.damage >= 50)
        }).filter((ele : ValorantMatchDamageWrapper) => {
            // Is the receiver of the damage on the kill feed?
            return (this.kills.findIndex((kele : ValorantMatchKillWrapper) => {
                return (kele.victim._p.puuid == ele.receiver._p.puuid)
                    && (kele.killer._p.puuid != puuid)
            }) != -1)
        }).length
    }

    getCombatScoreForPlayer(puuid : string) : number {
        return this.playerToStats[puuid].combatScore
    }

    getDamageDoneByPlayer(puuid : string) : number {
        return this.damage.filter((ele : ValorantMatchDamageWrapper) => {
            return (ele.instigator._p.puuid == puuid)
        }).reduce((acc, ele) => {
            return acc + ele._d.damage
        }, 0)
    }

    getLoadoutForPlayer(puuid : string) : ValorantMatchLoadout {
        return this.playerToLoadout[puuid]
    }

    get sortedKills() : ValorantMatchKillWrapper[] {
        return [...this.kills].sort((a : ValorantMatchKillWrapper, b : ValorantMatchKillWrapper) => {
            return a._k.roundTime - b._k.roundTime
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
        return (this._p.kills + this._p.assists) / this._p.deaths
    }

    get cspr() : number {
        return (this._p.roundsPlayed == 0) ? 0.0 : this._p.totalCombatScore / this._p.roundsPlayed
    }

    get dpr() : number {
        return (this._p.roundsPlayed == 0) ? 0.0 : this.damage.reduce((acc : number, curr : ValorantMatchDamageWrapper) => {
            return acc + curr._d.damage
        }, 0.0) / this._p.roundsPlayed
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
            if (kill._k.roundNum in killsPerRound) {
                killsPerRound[kill._k.roundNum] += 1
            } else {
                killsPerRound[kill._k.roundNum] = 1
            }
            maxRound = Math.max(kill._k.roundNum)
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

    constructor(t : ValorantMatchTeam) {
        this._t = t
        this.players = []
    }

    getPlayersDescendingCS() : ValorantMatchPlayerWrapper[] {
        return [...this.players].sort((a : ValorantMatchPlayerWrapper, b : ValorantMatchPlayerWrapper) => {
            return b._p.totalCombatScore - a._p.totalCombatScore
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
        for (let t of this._details.teams) {
            this._teams[t.teamId] = new ValorantMatchTeamWrapper(t)
        }

        this._players = {}
        for (let p of this._details.players) {
            let np = new ValorantMatchPlayerWrapper(p)
            this._teams[p.teamId].players.push(np)
            this._players[p.puuid] = np
        }

        this._rounds = this._details.rounds.map((ele : ValorantMatchRound) => {
            return new ValorantMatchRoundWrapper(ele)
        })

        this._kills = this._details.kills.map((ele : ValorantMatchKill) => {
            return new ValorantMatchKillWrapper(
                ele,
                this.getPlayer(ele.killer),
                this.getPlayer(ele.victim),
            )
        })

        this._kills.forEach((ele : ValorantMatchKillWrapper) => {
            this._rounds[ele._k.roundNum].kills.push(ele)
            this._players[ele.killer._p.puuid].kills.push(ele)
            this._players[ele.victim._p.puuid].deaths.push(ele)
        })

        this._damage = this._details.damage.map((ele : ValorantMatchDamage) => {
            return new ValorantMatchDamageWrapper(
                ele,
                this.getPlayer(ele.instigator),
                this.getPlayer(ele.receiver),
            )
        })

        this._damage.forEach((ele : ValorantMatchDamageWrapper) => {
            this._rounds[ele._d.roundNum].damage.push(ele)
            this._players[ele.instigator._p.puuid].damage.push(ele)
        })
    }

    getDamageDoneByPlayer(puuid : string) : number {
        return this._damage.filter((ele : ValorantMatchDamageWrapper) => {
            return (ele.instigator._p.puuid == puuid)
        }).reduce((acc, ele) => {
            return acc + ele._d.damage
        }, 0)
    }

    getPlayersDescendingCS() : ValorantMatchPlayerWrapper[] {
        return this._details.players.map((ele : ValorantMatchPlayer) => {
            return this._players[ele.puuid]
        }).sort((a : ValorantMatchPlayerWrapper, b : ValorantMatchPlayerWrapper) => {
            return b._p.totalCombatScore - a._p.totalCombatScore
        })
    }

    getMatchStartTime() : Date {
        if (!!this._details.ovStartTime) {
            return this._details.ovStartTime
        } else {
            return this._details.matchTime
        }
    }

    getPlayer(puuid : string) : ValorantMatchPlayerWrapper {
        return this._players[puuid]
    }

    getPlayerAgentId(puuid : string) : string {
        return this.getPlayer(puuid)._p.agentId
    }

    getPlayerAgentName(puuid : string) : string {
        let cnt = getValorantContent(this._details.patchId)
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

            if (ele.sortedKills[0].killer._p.puuid == puuid) {
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