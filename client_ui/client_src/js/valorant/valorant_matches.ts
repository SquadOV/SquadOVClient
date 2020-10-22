import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import { getValorantContent } from '@client/js/valorant/valorant_content'

export interface ValorantMatchTeam {
    teamId: string
    won: boolean
    roundsWon: number
    roundsPlayed: number
}

export interface ValorantMatchPlayer {
    puuid : string
    teamId: string
    agentId: string
    competitiveTier: number
    totalCombatScore: number
    roundsPlayed: number
    kills: number
    deaths: number
    assists: number
}

export interface ValorantMatchLoadout {
    roundNum : number
    puuid: string
    loadoutValue: number
    remainingMoney : number
    spentMoney : number
    weapon: string
    armor: string | null
}

export interface ValorantMatchPlayerRoundStat {
    roundNum : number
    puuid: string
    combatScore : number
}

export interface ValorantMatchRound {
    startBuyTime : Date | null
    startPlayTime: Date | null

    roundNum : number
    plantRoundTime : number | null
    planter: string | null

    defuseRoundTime: number | null
    defuser : string | null

    roundWinner: string
    loadouts: ValorantMatchLoadout[]
    roundStats: ValorantMatchPlayerRoundStat[]
}

export interface ValorantMatchKill {
    roundNum : number
    roundTime: number
    damageType: string
    damageItem: string
    secondaryFire: boolean
    killer: string
    victim: string
}

export interface ValorantMatchDamage {
    roundNum : number
    damage: number
    headshots: number
    bodyshots: number
    legshots: number
    instigator: string
    receiver: string
}

export interface ValorantMatchDetails {
    matchId : string
    matchTime : Date
    ovStartTime: Date | null
    ovEndTime: Date | null    
    map : string
    provisioningFlowId : string
    gameMode : string
    patchId : string
    isRanked: boolean
    vodPath : string | null

    players: ValorantMatchPlayer[]
    teams: ValorantMatchTeam[]
    rounds: ValorantMatchRound[]
    kills: ValorantMatchKill[]
    damage: ValorantMatchDamage[]
}

export function cleanValorantMatchRound(m :ValorantMatchRound) : ValorantMatchRound {
    if (!!m.startBuyTime) {
        m.startBuyTime = new Date(m.startBuyTime)
    }

    if (!!m.startPlayTime ){
        m.startPlayTime = new Date(m.startPlayTime)
    }
    return m
}

export function cleanValorantMatchDetails(m :ValorantMatchDetails) : ValorantMatchDetails {
    m.matchTime = new Date(m.matchTime)

    if (!!m.ovStartTime) {
        m.ovStartTime = new Date(m.ovStartTime)
    }

    if (!!m.ovEndTime) {
        m.ovEndTime = new Date(m.ovEndTime)
    }

    m.rounds.forEach(cleanValorantMatchRound)
    return m
}

export interface ValorantPlayerMatchSummary {
    matchId : string
    serverStartTimeUtc: Date | null
    gameMode: string | null
    map: string | null
    isRanked: boolean | null
    provisioningFlowID: string | null
    gameVersion: string | null
    characterId: string
    won: boolean
    roundsWon: number
    roundsLost: number
    combatScoreRank: number
    competitiveTier: number
    kills: number
    deaths: number
    assists: number
    roundsPlayed: number
    totalCombatScore: number
    totalDamage: number
    headshots: number
    bodyshots: number
    legshots: number
    hasVod: boolean
}

export function cleanValorantPlayerMatchSummary(m :ValorantPlayerMatchSummary) : ValorantPlayerMatchSummary {
    if (!!m.serverStartTimeUtc) {
        m.serverStartTimeUtc = new Date(m.serverStartTimeUtc)
    }
    return m
}

export function getGameMode(patchId : string | null, gameMode: string | null, isRanked : boolean | null) : string | null {
    if (!patchId || !gameMode) {
        return null
    }

    let cnt = getValorantContent(patchId)
    let mode = cnt.gameModeToName(gameMode)
    if (mode == "Standard") {
        if (!!isRanked) {
            return "Competitive"
        } else {
            return "Unrated"
        }
    } else {
        return mode
    }
}

export function getIsCustom(provisioningFlowId : string) : boolean {
    return provisioningFlowId === 'CustomGame'
}