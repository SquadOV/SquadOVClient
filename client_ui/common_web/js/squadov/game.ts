export enum SquadOvGames {
    AimLab,
    Hearthstone,
    LeagueOfLegends,
    TeamfightTactics,
    Valorant,
    WorldOfWarcraft,
    Csgo,
    Unknown,
}

export let allGames = [
    SquadOvGames.AimLab,
    SquadOvGames.Hearthstone,
    SquadOvGames.LeagueOfLegends,
    SquadOvGames.TeamfightTactics,
    SquadOvGames.Valorant,
    SquadOvGames.WorldOfWarcraft,
    SquadOvGames.Csgo
]

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
        case SquadOvGames.Csgo:
            return 'assets/csgo-logo.png'
        case SquadOvGames.Unknown:
            return ''
    }
}

export function gameToName(game: SquadOvGames): string {
    switch (game) {
        case SquadOvGames.AimLab:
            return 'Aim Lab'
        case SquadOvGames.Hearthstone:
            return 'Hearthstone'
        case SquadOvGames.LeagueOfLegends:
            return 'League of Legends'
        case SquadOvGames.TeamfightTactics:
            return 'Teamfight Tactics'
        case SquadOvGames.Valorant:
            return 'Valorant'
        case SquadOvGames.WorldOfWarcraft:
            return 'World of Warcraft'
        case SquadOvGames.Csgo:
            return 'CS:GO'
        case SquadOvGames.Unknown:
            return 'Unknown'
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
        case 'csgo':
            return SquadOvGames.Csgo
    }
    return SquadOvGames.Unknown
}