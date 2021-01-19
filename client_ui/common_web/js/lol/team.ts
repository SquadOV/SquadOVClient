export interface LolTeamStats {
    towerKills: number
    riftHeraldKills: number
    firstBlood: boolean
    inhibitorKills: number
    firstBaron: boolean
    firstDragon: boolean
    dragonKills: number
    baronKills: number
    firstInhibitor: boolean
    firstTower: boolean
    firstRiftHerald: boolean
    teamId: number
    win: string
    bans: LolTeamBan[]
}

export interface LolTeamBan {
    championId: number
    pickTurn: number
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