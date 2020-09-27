import { 
    ValorantMatchDetails,
    ValorantMatchTeam,
    ValorantMatchPlayer,
    ValorantMatchRound,
    ValorantMatchKill
} from '@client/js/valorant/valorant_matches'

export class ValorantMatchKillWrapper {
    _k : ValorantMatchKill

    killer: ValorantMatchPlayerWrapper
    victim: ValorantMatchPlayerWrapper

    constructor(k : ValorantMatchKill, killer : ValorantMatchPlayerWrapper, victim : ValorantMatchPlayerWrapper) {
        this._k = k
        this.killer = killer
        this.victim = victim
    }
}

export class ValorantMatchRoundWrapper {
    _r : ValorantMatchRound
    kills : ValorantMatchKillWrapper[]

    constructor(r : ValorantMatchRound) {
        this._r = r
        this.kills = []
    }
}

export class ValorantMatchPlayerWrapper {
    _p : ValorantMatchPlayer
    constructor(p : ValorantMatchPlayer) {
        this._p = p
    }
}

interface MatchPlayerMap {
    [ puuid : string ] : ValorantMatchPlayerWrapper
}

export class ValorantMatchTeamWrapper {
    _t : ValorantMatchTeam
    constructor(t : ValorantMatchTeam) {
        this._t = t
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

    constructor(details : ValorantMatchDetails) {
        this._details = details

        this._teams = {}
        for (let t of this._details.teams) {
            this._teams[t.teamId] = new ValorantMatchTeamWrapper(t)
        }

        this._players = {}
        for (let p of this._details.players) {
            this._players[p.puuid] = new ValorantMatchPlayerWrapper(p)
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

    getPlayerTeam(puuid : string) : ValorantMatchTeamWrapper {
        let p = this._players[puuid]
        return this._teams[p._p.teamId]
    }

    getRound(rnd : number) : ValorantMatchRoundWrapper | null {
        if (rnd < 0 || rnd >= this._rounds.length) {
            return null
        }
        return this._rounds[rnd]
    }
}