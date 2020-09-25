export interface ValorantPlayerStatsSummary {
    rank : number
    kills : number
    deaths : number
    assists : number
    rounds : number
    totalCombatScore : number
    totalDamage : number
    headshots: number
    bodyshots : number
    legshots : number
    wins: number
    games: number
}

export function kda(s : ValorantPlayerStatsSummary) {
    return (s.kills + s.assists) / s.deaths
}

export function hsp(s : ValorantPlayerStatsSummary) {
    return (s.headshots) / (s.headshots + s.bodyshots + s.legshots)
}

export function dpr(s : ValorantPlayerStatsSummary) : number {
    if (s.rounds == 0) {
        return 0
    }
    return s.totalDamage / s.rounds
}

export function cspr(s : ValorantPlayerStatsSummary) : number {
    if (s.rounds == 0) {
        return 0
    }
    return s.totalCombatScore / s.rounds
}