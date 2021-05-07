export interface CsgoPlayerMatchSummary {
    matchUuid: string
    userUuid: string
    map: string
    mode: string
    matchStartTime: Date
    matchLengthSeconds: number
    kills: number
    deaths: number
    assists: number
    mvps: number
    winner: boolean
    hasDemo: boolean
    headshots: number
    bodyshots: number
    legshots: number
    damagePerRound: number
    friendlyRounds: number
    enemyRounds: number
}

export function cleanCsgoPlayerMatchSummaryFromJson(s: CsgoPlayerMatchSummary): CsgoPlayerMatchSummary {
    s.matchStartTime = new Date(s.matchStartTime)
    return s
}