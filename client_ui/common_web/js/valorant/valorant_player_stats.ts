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

export function kda(kills: number, assists: number, deaths: number) : number {
    return (kills + assists) / deaths
}

export function hsp(headshots: number, bodyshots: number, legshots: number) : number {
    return headshots/ (headshots + bodyshots + legshots)
}

export function dpr(totalDamage: number, rounds: number) : number {
    if (rounds == 0) {
        return 0
    }
    return totalDamage / rounds
}

export function cspr(combatScore: number, rounds: number) : number {
    if (rounds == 0) {
        return 0
    }
    return combatScore / rounds
}