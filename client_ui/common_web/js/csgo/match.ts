import { CsgoPlayerMatchSummary, cleanCsgoPlayerMatchSummaryFromJson } from '@client/js/csgo/summary'
import { CsgoView, cleanCsgoViewFromJson } from '@client/js/csgo/view'
import {
    CsgoEventContainer,
    CsgoEventRound,
    cleanCsgoEventContainerFromJson,
    CsgoTeam,
    CsgoRoundPlayerStats,
    CsgoEventPlayer,
    CsgoRoundKill,
    CsgoBombStatus,
} from '@client/js/csgo/events'
import { SteamAccount } from '@client/js/steam/account'

export interface CsgoFullMatchData {
    summary: CsgoPlayerMatchSummary
    view: CsgoView
    container: CsgoEventContainer
}

export function cleanCsgoFullMatchDataFromJson(d: CsgoFullMatchData): CsgoFullMatchData {
    cleanCsgoPlayerMatchSummaryFromJson(d.summary)
    cleanCsgoViewFromJson(d.view)
    cleanCsgoEventContainerFromJson(d.container)
    return d
}

export class CsgoEventRoundWrapper {
    _r: CsgoEventRound

    _userStats: Map<number, CsgoRoundPlayerStats>
    _userKills: Map<number, CsgoRoundKill[]>
    _userDeaths: Map<number, CsgoRoundKill[]>
    _userAssists: Map<number, CsgoRoundKill[]>

    get hasBombPlant(): boolean {
        return !!this._r.tmBombPlant && this._r.bombPlantSite !== null && this._r.bombPlantUser !== null
    }

    get hasBombDefuse(): boolean {
        return !!this._r.tmBombEvent && this._r.bombEventUser !== null && this._r.bombState == CsgoBombStatus.Defused
    }

    get hasBombExplode(): boolean {
        return !!this._r.tmBombEvent && this._r.bombState == CsgoBombStatus.Exploded
    }

    orderedTeamPlayers(t: CsgoTeam): number[] {
        return Array.from(this._userStats.values())
            .filter((ele: CsgoRoundPlayerStats) => {
                return ele.team === t
            })
            .sort((a: CsgoRoundPlayerStats, b: CsgoRoundPlayerStats) => {
                // we want a consistent ordering between rounds.
                return a.userId - b.userId
            })
            .map((ele: CsgoRoundPlayerStats) => {
                return ele.userId
            })
    }

    addKill(u: number, k: CsgoRoundKill) {
        if (!this._userKills.has(u)) {
            this._userKills.set(u, [])
        }

        this._userKills.get(u)!.push(k)
    }

    addDeath(u: number, k: CsgoRoundKill) {
        if (!this._userDeaths.has(u)) {
            this._userDeaths.set(u, [])
        }

        this._userDeaths.get(u)!.push(k)
    }

    addAssist(u: number, k: CsgoRoundKill) {
        if (!this._userAssists.has(u)) {
            this._userAssists.set(u, [])
        }

        this._userAssists.get(u)!.push(k)
    }

    constructor(r: CsgoEventRound) {
        this._r = r

        this._userStats = new Map()
        this._userKills = new Map()
        this._userDeaths = new Map()
        this._userAssists = new Map()

        for (let u of this._r.playerStats) {
            this._userStats.set(u.userId, u)
        }

        for (let k of this._r.kills) {
            if (k.killer !== null) {
                this.addKill(k.killer, k)
            }

            if (k.assister !== null) {
                this.addAssist(k.assister, k)
            }

            if (k.victim !== null) {
                this.addDeath(k.victim, k)
            }
        }
    }

    userTeam(u: number): CsgoTeam {
        let s = this.userStats(u)
        if (!s) {
            return CsgoTeam.Spectate
        }
        return s.team
    }

    userStats(u: number): CsgoRoundPlayerStats | undefined {
        return this._userStats.get(u)
    }

    get roundNum(): number {
        return this._r.roundNum
    }

    isUserWinner(u: number): boolean {
        return this._r.winningTeam === this.userTeam(u)
    }

    isUserMvp(u: number): boolean {
        return this._r.roundMvp === u
    }

    didUserDie(u: number): boolean {
        let deaths = this._userDeaths.get(u)
        if (!deaths) {
            return false
        }
        return deaths.length > 0
    }
}

export class CsgoFullMatchDataWrapper {
    _d: CsgoFullMatchData
    _wrappedRounds: Map<number, CsgoEventRoundWrapper>
    _playerMap: Map<number, CsgoEventPlayer>
    _userIdToSteamId: Map<number, number>

    constructor(d: CsgoFullMatchData) {
        this._d = d

        this._wrappedRounds = new Map()
        for (let r of this._d.container.rounds) {
            this._wrappedRounds.set(r.roundNum, new CsgoEventRoundWrapper(r))
        }

        this._playerMap = new Map()
        this._userIdToSteamId = new Map()

        for (let p of this._d.container.players) {
            this._playerMap.set(p.steamAccount.steamId, p)
            this._userIdToSteamId.set(p.userId, p.steamAccount.steamId)
        }
    }

    get rounds(): CsgoEventRoundWrapper[] {
        return this._d.container.rounds.map((ele: CsgoEventRound) => {
            return this._wrappedRounds.get(ele.roundNum)!
        })
    }

    round(num: number): CsgoEventRoundWrapper | undefined {
        return this._wrappedRounds.get(num)
    }

    // Use the last round to obtain team information on who's friendly and who's not.
    friendlySteamIds(inputSteamId: number): number[] {
        if (this.rounds.length === 0) {
            return []
        }
        let userId = this.steamIdToUserId(inputSteamId)
        if (userId === null) {
            return []
        }

        let lastRound = this.rounds[this.rounds.length - 1]
        let wrapper = this._wrappedRounds.get(lastRound.roundNum)!
        let team = wrapper.userTeam(userId)
        return <number[]>lastRound._r.playerStats
            .filter((ele: CsgoRoundPlayerStats) => ele.team === team)
            .map((ele: CsgoRoundPlayerStats) => {
                return this.userIdToSteamId(ele.userId)
            })
            .filter((ele: number| null) => ele !== null)
    }

    enemySteamIds(inputSteamId: number): number[] {
        if (this.rounds.length === 0) {
            return []
        }
        let userId = this.steamIdToUserId(inputSteamId)
        if (userId === null) {
            return []
        }

        let lastRound = this.rounds[this.rounds.length - 1]
        let wrapper = this._wrappedRounds.get(lastRound.roundNum)!
        let team = wrapper.userTeam(userId)
        return <number[]>lastRound._r.playerStats
            .filter((ele: CsgoRoundPlayerStats) => ele.team !== team)
            .map((ele: CsgoRoundPlayerStats) => {
                return this.userIdToSteamId(ele.userId)
            })
            .filter((ele: number| null) => ele !== null)
    }

    steamAccount(steamId: number | null): SteamAccount | undefined {
        if (steamId === null) {
            return undefined
        }
        return this._playerMap.get(steamId)?.steamAccount
    }

    steamIdToUserId(steamId: number): number | null {
        let p = this._playerMap.get(steamId)
        if (!p) {
            return null
        }
        return p.userId
    }

    userIdToSteamId(userId: number): number | null {
        let s = this._userIdToSteamId.get(userId)
        if (s === undefined) {
            return null
        }
        return s
    }
}