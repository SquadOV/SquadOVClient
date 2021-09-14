export interface LolTeamStats {
    teamId: number
    win: string
    bans: LolTeamBan[]
    objectives: LolObjectives
}

export interface LolTeamBan {
    championId: number
    pickTurn: number
}

export interface LolObjectives {
    baron: LolSingleObjective
    champion: LolSingleObjective
    dragon: LolSingleObjective
    inhibitor: LolSingleObjective
    riftHerald: LolSingleObjective
    tower: LolSingleObjective
}

export interface LolSingleObjective {
    first: boolean
    kills: number
}

export function lolTeamIdToString(id: number): string {
    if (id == 100) {
        return 'Blue'
    } else {
        return 'Red'
    }
}

export function getOpposingLolTeam(id: number): number {
    if (id == 100) {
        return 200
    } else {
        return 100
    }
}