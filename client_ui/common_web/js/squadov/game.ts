export enum SquadOvGames {
    AimLab,
    Hearthstone,
    LeagueOfLegends,
    TeamfightTactics,
    Valorant,
    WorldOfWarcraft,
    Csgo,
    WowVanilla,
    WowTbc,
    Ff14,
    Unknown,
}

export let allGames = [
    SquadOvGames.AimLab,
    SquadOvGames.Hearthstone,
    SquadOvGames.LeagueOfLegends,
    SquadOvGames.TeamfightTactics,
    SquadOvGames.Valorant,
    SquadOvGames.WorldOfWarcraft,
    SquadOvGames.Csgo,
    // these two are client side only
    SquadOvGames.WowVanilla,
    SquadOvGames.WowTbc,
    //SquadOvGames.Ff14,
]

export let allServerSideGames = [
    SquadOvGames.AimLab,
    SquadOvGames.Hearthstone,
    SquadOvGames.LeagueOfLegends,
    SquadOvGames.TeamfightTactics,
    SquadOvGames.Valorant,
    SquadOvGames.WorldOfWarcraft,
    SquadOvGames.Csgo,
    //SquadOvGames.Ff14,
]

export function gameToRect(game: SquadOvGames): string {
    switch (game) {
        case SquadOvGames.AimLab:
            return 'assets/games/aimlab_rect_small.jpg'
        case SquadOvGames.Hearthstone:
            return 'assets/games/hearthstone_rect_small.jpg'
        case SquadOvGames.LeagueOfLegends:
            return 'assets/games/lol_rect_small.jpg'
        case SquadOvGames.TeamfightTactics:
            return 'assets/games/tft_rect_small.jpg'
        case SquadOvGames.Valorant:
            return 'assets/games/valorant_rect_small.jpg'
        case SquadOvGames.WorldOfWarcraft:
            return 'assets/games/wow_rect_small.jpg'
        case SquadOvGames.Csgo:
            return 'assets/games/csgo_rect_small.jpg'
        case SquadOvGames.WowVanilla:
            return 'assets/games/wow_vanilla_rect_small.jpg'
        case SquadOvGames.WowTbc:
            return 'assets/games/wow_tbc_rect_small.jpg'
        case SquadOvGames.Ff14:
            return 'assets/games/ff14_rect_small.jpg'
        case SquadOvGames.Unknown:
            return ''
    }
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
        case SquadOvGames.Csgo:
            return 'assets/csgo-logo.png'
        case SquadOvGames.WowVanilla:
            return 'assets/wowc-logo.png'
        case SquadOvGames.WowTbc:
            return 'assets/tbc-logo.png'
        case SquadOvGames.Ff14:
            return 'assets/ff14-logo'
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
        case SquadOvGames.WowVanilla:
            return 'World of Warcraft Classic'
        case SquadOvGames.WowTbc:
            return 'Burning Crusade Classic'
        case SquadOvGames.Ff14:
            return 'Final Fantasy 14'
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
        case 'wowc':
            return SquadOvGames.WowVanilla
        case 'tbc':
            return SquadOvGames.WowTbc
        case 'ff14':
            return SquadOvGames.Ff14
    }
    return SquadOvGames.Unknown
}

// This is...slightly duplicating of SquadOvGames.
// The main reason we need this is because I made SquadOvGames mean different things so we can't
// actually send the client side WowVanilla and WowTbc...uhh..woops. Fight me. :D
export enum SquadOvWowRelease {
    Retail,
    Vanilla,
    Tbc,
}

export const ALL_WOW_RELEASES = [
    SquadOvWowRelease.Retail,
    SquadOvWowRelease.Vanilla,
    SquadOvWowRelease.Tbc,
]

export function wowReleaseToGame(r: SquadOvWowRelease): SquadOvGames {
    switch (r) {
        case SquadOvWowRelease.Retail:
            return SquadOvGames.WorldOfWarcraft
        case SquadOvWowRelease.Vanilla:
            return SquadOvGames.WowVanilla
        case SquadOvWowRelease.Tbc:
            return SquadOvGames.WowTbc
    }
}

export function wowReleaseToName(r: SquadOvWowRelease): string {
    switch (r) {
        case SquadOvWowRelease.Retail:
            return 'Retail'
        case SquadOvWowRelease.Vanilla:
            return 'Vanilla'
        case SquadOvWowRelease.Tbc:
            return 'Burning Crusade'
    }
}

export function gameToPossibleWowTuple(game: SquadOvGames) : [SquadOvGames, SquadOvWowRelease | null] {
    switch (game) {
        default:
            return [game, null]
        case SquadOvGames.WorldOfWarcraft:
            return [SquadOvGames.WorldOfWarcraft, SquadOvWowRelease.Retail]
        case SquadOvGames.WowVanilla:
            return [SquadOvGames.WorldOfWarcraft, SquadOvWowRelease.Vanilla]
        case SquadOvGames.WowTbc:
            return [SquadOvGames.WorldOfWarcraft, SquadOvWowRelease.Tbc]
    }
}

export interface FullSupportedGame {
    game: SquadOvGames
    wow: SquadOvWowRelease | null
}

export function fullSupportGameToEnum(g: FullSupportedGame): SquadOvGames {
    switch (g.game) {
        case SquadOvGames.WorldOfWarcraft:
            if (g.wow === null) {
                return SquadOvGames.WorldOfWarcraft
            } else {
                switch (g.wow) {
                    case SquadOvWowRelease.Retail:
                        return SquadOvGames.WorldOfWarcraft
                    case SquadOvWowRelease.Vanilla:
                        return SquadOvGames.WowVanilla
                    case SquadOvWowRelease.Tbc:
                        return SquadOvGames.WowTbc
                }
            }
        default:
            return g.game
    }
}