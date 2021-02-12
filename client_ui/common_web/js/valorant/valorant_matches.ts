import { getValorantContent } from '@client/js/valorant/valorant_content'

export interface ValorantMatchInfo {
    matchId: string
    mapId: string | null
    gameLengthMillis: number
    serverStartTimeUtc: Date
    provisioningFlowId: string | null
    gameMode: string | null
    isRanked: boolean | null
    seasonId: string | null
}

export function cleanValorantMatchInfoFromJson(m: ValorantMatchInfo): ValorantMatchInfo {
    if (!!m.serverStartTimeUtc) {
        m.serverStartTimeUtc = new Date(m.serverStartTimeUtc)
    }
    return m
}

export interface ValorantMatchPlayer {
    puuid: string
    teamId: string
    characterId: string
    stats: ValorantMatchPlayerStats
    competitiveTier: number
}

export interface ValorantMatchPlayerStats {
    score: number
    roundsPlayed: number
    kills: number
    deaths: number
    assists: number
}

export interface ValorantMatchTeam {
    teamId: string
    won: boolean
    roundsPlayed: number
    roundsWon: number
    numPoints: number
}

export interface ValorantMatchRoundResult {
    roundNum: number
    winningTeam: string
    bombPlanter: string | null
    bombDefuser: string | null
    plantRoundTime: number | null
    defuseRoundTime: number | null
    playerStats: ValorantMatchPlayerRoundStats[]
}

export interface ValorantMatchKill {
    timeSinceGameStartMillis: number
    timeSinceRoundStartMillis: number
    killer: string | null
    victim: string
    assistants: string[]
    finishingDamage: ValorantMatchFinishingDamage
}

export interface ValorantMatchFinishingDamage {
    damageType: string
    damageItem: string
    isSecondaryFireMode: boolean
}

export interface ValorantMatchDamage {
    receiver: string
    damage: number
    legshots: number
    bodyshots: number
    headshots: number
}

export interface ValorantMatchEconomy {
    loadoutValue: number
    weapon: string
    armor: string
    remaining: number
    spent: number
}

export interface ValorantMatchPlayerRoundStats {
    puuid: string
    kills: ValorantMatchKill[]
    damage: ValorantMatchDamage[]
    economy: ValorantMatchEconomy
    score: number
}

export interface ValorantMatch {
    matchInfo: ValorantMatchInfo
    players: ValorantMatchPlayer[]
    teams: ValorantMatchTeam[]
    roundResults: ValorantMatchRoundResult[]
}

export function cleanValorantMatchFromJson(m: ValorantMatch) : ValorantMatch {
    cleanValorantMatchInfoFromJson(m.matchInfo)
    return m
}

export interface ValorantMatchDetails {
    uuid: string
    data: ValorantMatch
}

export function cleanValorantMatchDetails(m :ValorantMatchDetails) : ValorantMatchDetails {
    cleanValorantMatchFromJson(m.data)
    return m
}

export interface ValorantPlayerMatchSummary {
    matchId : string
    matchUuid: string
    serverStartTimeUtc: Date | null
    gameMode: string | null
    mapId: string | null
    isRanked: boolean | null
    provisioningFlowId: string | null
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
    puuid: string
}

export function cleanValorantPlayerMatchSummary(m :ValorantPlayerMatchSummary) : ValorantPlayerMatchSummary {
    if (!!m.serverStartTimeUtc) {
        m.serverStartTimeUtc = new Date(m.serverStartTimeUtc)
    }
    return m
}

export function getGameMode(gameMode: string | null, isRanked : boolean | null) : string | null {
    if (!gameMode) {
        return null
    }

    let cnt = getValorantContent(null)
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

export function getIsDeathmatch(gameMode: string | null): boolean {
    if (!gameMode) {
        return false
    }

    let cnt = getValorantContent(null)
    let mode = cnt.gameModeToName(gameMode)
    return mode === 'Deathmatch'
}

export interface ValorantMatchPlayerRoundMetadata {
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
