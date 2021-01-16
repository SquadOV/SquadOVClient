export interface LolPlayerMatchSummary {
    matchUuid: string
    gameCreation: Date
    gameDuration: number
    gameType: string
    gameVersion: string
    queueId: number
    seasonId: number
    mapId: number
    gameMode: string
    currentParticipantId: number
    participants: LolMiniParticipantStats[]
    hasVod: boolean
}

export interface LolMiniParticipantStats {
    participantId: number
    championId: number
    summonerName: string | null
    teamId: number
    kills: number
    deaths: number
    assists: number
    totalDamageDealt: number
    totalMinionsKilled: number
    wardsPlaced: number
    lane: string
    win: boolean
}

export function cleanLolPlayerMatchSummaryFromJson(s: LolPlayerMatchSummary): LolPlayerMatchSummary {
    s.gameCreation = new Date(s.gameCreation)
    return s
}