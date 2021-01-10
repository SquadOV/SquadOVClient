import { getValorantContent } from '@client/js/valorant/valorant_content'

export interface ValorantMatchTeam {
    teamId: string
    won: boolean
    roundsWon: number
    roundsPlayed: number
}

export interface ValorantPlayerStats {
    score: number
    roundsPlayed: number
    kills: number
    deaths: number
    assists: number
}

export interface ValorantMatchPlayer {
    subject: string,
    characterId: string
    competitiveTier: number
    teamId: string
    stats: ValorantPlayerStats
}

export interface ValorantMatchLoadout {
    subject: string
    armor: string
    weapon: string
    remaining: number
    loadoutValue: number
    spent: number
}

export interface ValorantMatchDamage {
    receiver: string
    damage: number
    legshots: number
    bodyshots: number
    headshots: number
}

export interface ValorantMatchPlayerRoundStat {
    subject: string
    score: number
    damage: ValorantMatchDamage[]
}

export interface ValorantMatchRound {
    roundNum: number
    plantRoundTime: number | null
    bombPlanter: string | null
    defuseRoundTime: number | null
    bombDefuser: string | null
    winningTeam: string
    playerStats: ValorantMatchPlayerRoundStat[]
    playerEconomies: ValorantMatchLoadout[] | null
}

export interface ValorantFinishingDamage {
    damageType: string
    damageItem: string
    isSecondaryFireMode: boolean
}

export interface ValorantMatchKill {
    roundTime: number
    round: number
    finishingDamage: ValorantFinishingDamage
    killer: string | null
    victim: string
}

export interface ValorantMatchMetadata {
    matchId: string
    gameMode: string | null
    mapId: string | null
    isRanked: boolean | null
    provisioningFlowID: string | null
    gameVersion: string | null
    serverStartTimeUtc: Date | null
}

export interface ValorantMatchDetails {
    matchUuid: string
    matchInfo: ValorantMatchMetadata
    teams: ValorantMatchTeam[]
    players: ValorantMatchPlayer[]
    roundResults: ValorantMatchRound[]
    kills: ValorantMatchKill[]
    rawData: any
}

export function cleanValorantMatchDetails(m :ValorantMatchDetails) : ValorantMatchDetails {
    if (!!m.matchInfo.serverStartTimeUtc) {
        m.matchInfo.serverStartTimeUtc = new Date(m.matchInfo.serverStartTimeUtc)
    }
    return m
}

export interface ValorantPlayerMatchSummary {
    matchId : string
    matchUuid: string
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

export function getIsCustom(provisioningFlowId : string | null) : boolean {
    return provisioningFlowId === 'CustomGame'
}

export function getIsDeathmatch(patchId : string | null, gameMode: string | null): boolean {
    if (!patchId || !gameMode) {
        return false
    }

    let cnt = getValorantContent(patchId)
    let mode = cnt.gameModeToName(gameMode)
    return mode === 'Deathmatch'
}

export interface ValorantMatchPlayerRoundMetadata {
    matchId: string
    puuid: string
    round: number
    buyTime: Date | null
    roundTime: Date | null
}

export function cleanValorantMatchPlayerRoundMetadata(v : ValorantMatchPlayerRoundMetadata) : ValorantMatchPlayerRoundMetadata {
    if (!!v.buyTime) {
        v.buyTime = new Date(v.buyTime)
    }

    if (!!v.roundTime) {
        v.roundTime = new Date(v.roundTime)
    }
    return v
}

export interface ValorantMatchPlayerMatchMetadata {
    matchId: string
    puuid: string
    startTime: Date
    endTime: Date
    rounds: ValorantMatchPlayerRoundMetadata[]
}

export function cleanValorantMatchPlayerMatchMetadata(v: ValorantMatchPlayerMatchMetadata) : ValorantMatchPlayerMatchMetadata {
    v.startTime = new Date(v.startTime)
    v.endTime = new Date(v.endTime)
    v.rounds.forEach(cleanValorantMatchPlayerRoundMetadata)
    return v
}
