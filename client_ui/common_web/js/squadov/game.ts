export enum SquadOvGames {
    AimLab,
    Hearthstone,
    LeagueOfLegends,
    TeamfightTactics,
    Valorant,
    WorldOfWarcraft,
}

export function gameToIcon(game: SquadOvGames): string {
    switch (game) {
        case SquadOvGames.AimLab:
            return 'assets/aimlab-logo.png'
        case SquadOvGames.Hearthstone:
            return 'assets/hearthstone-logo.png'
        case SquadOvGames.LeagueOfLegends:
            return 'assets/lol-logo.png'
        case SquadOvGames.TeamfightTactics:
            return 'assets/tft-logo.png'
        case SquadOvGames.Valorant:
            return 'assets/valorant-logo.png'
        case SquadOvGames.WorldOfWarcraft:
            return 'assets/wow-logo.png'
    }
}

export function gameShorthandToGame(str: string): SquadOvGames {
    switch (str) {
        case 'aimlab':
            return SquadOvGames.AimLab
        case 'hearthstone':
            return SquadOvGames.Hearthstone
        case 'lol':
            return SquadOvGames.LeagueOfLegends
        case 'tft':
            return SquadOvGames.TeamfightTactics
        case 'valorant':
            return SquadOvGames.Valorant
        case 'wow':
            return SquadOvGames.WorldOfWarcraft
    }
    return SquadOvGames.AimLab
}