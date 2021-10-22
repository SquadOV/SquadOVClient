import { SquadOvGames, SquadOvWowRelease } from '@client/js/squadov/game'
import { VodManifest } from '@client/js/squadov/vod'
import { AimlabTaskData, cleanAimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { LolPlayerMatchSummary, cleanLolPlayerMatchSummaryFromJson } from '@client/js/lol/matches'
import { TftPlayerMatchSummary, cleanTftPlayerMatchSummaryFromJson } from '@client/js/tft/matches'
import { WowChallenge, WowEncounter, WowArena, cleanWowChallengeFromJson, cleanWowEncounterFromJson, cleanWowArenaFromJson } from '@client/js/wow/matches'
import { ValorantPlayerMatchSummary, cleanValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'
import { CsgoPlayerMatchSummary, cleanCsgoPlayerMatchSummaryFromJson } from '@client/js/csgo/summary'
import { WowMatchFilters, createEmptyWowMatchFilters } from '@client/js/wow/filters'

export interface BaseRecentMatch {
    matchUuid: string
    isLocal: boolean
    tm: Date
    game: SquadOvGames
    vod: VodManifest
    username: string
    userId: number
    favoriteReason: string | null
    isWatchlist: boolean
    accessToken: string | null
}

export interface RecentMatch {
    base: BaseRecentMatch

    aimlabTask?: AimlabTaskData
    // We can skip hearthstone since the summary Vue component pulls the data itself
    lolMatch?: LolPlayerMatchSummary
    tftMatch?: TftPlayerMatchSummary
    valorantMatch?: ValorantPlayerMatchSummary
    wowChallenge?: WowChallenge
    wowEncounter?: WowEncounter
    wowArena?: WowArena
    csgoMatch?: CsgoPlayerMatchSummary
}

export interface MatchFavoriteResponse {
    favorite: boolean
    reason: string | null
}

export interface WowGenericMatchFilters {
    encounters: WowMatchFilters
    keystones: WowMatchFilters
    arenas: WowMatchFilters
}

function createEmptyWowGenericMatchFilters(): WowGenericMatchFilters {
    return {
        encounters: createEmptyWowMatchFilters(),
        keystones: createEmptyWowMatchFilters(),
        arenas: createEmptyWowMatchFilters(),
    }
}

export interface RecentMatchFilters {
    games: SquadOvGames[] | undefined
    wowReleases: SquadOvWowRelease[] | undefined
    squads: number[] | undefined
    users: number[] | undefined
    timeStart: number | undefined
    timeEnd: number | undefined
    onlyFavorite: boolean
    onlyWatchlist: boolean
    filters: {
        wow: WowGenericMatchFilters
    }
}

export function createEmptyRecentMatchFilters(): RecentMatchFilters {
    return {
        games: undefined,
        wowReleases: undefined,
        squads: undefined,
        users: undefined,
        timeStart: undefined,
        timeEnd: undefined,
        onlyFavorite: false,
        onlyWatchlist: false,
        filters: {
            wow: createEmptyWowGenericMatchFilters(),
        }
    }
}

export function checkRecentMatchValidity(r: RecentMatch): boolean {
    switch (r.base.game) {
        case SquadOvGames.AimLab:
            return !!r.aimlabTask
        case SquadOvGames.WorldOfWarcraft:
        case SquadOvGames.WowVanilla:
        case SquadOvGames.WowTbc:
            return !!r.wowChallenge || !!r.wowEncounter || !!r.wowArena
        case SquadOvGames.TeamfightTactics:
            return !!r.tftMatch
        case SquadOvGames.LeagueOfLegends:
            return !!r.lolMatch
        case SquadOvGames.Valorant:
            return !!r.valorantMatch
        case SquadOvGames.Hearthstone:
            return true
        case SquadOvGames.Csgo:
            return !!r.csgoMatch
        case SquadOvGames.Unknown:
            return false
    }
}

export function cleanRecentMatchFromJson(r: RecentMatch): RecentMatch {
    r.base.tm = new Date(r.base.tm)

    if (!!r.aimlabTask) {
        cleanAimlabTaskData(r.aimlabTask)
    }

    if (!!r.lolMatch) {
        cleanLolPlayerMatchSummaryFromJson(r.lolMatch)
    }

    if (!!r.tftMatch) {
        cleanTftPlayerMatchSummaryFromJson(r.tftMatch)
    }

    if (!!r.valorantMatch) {
        cleanValorantPlayerMatchSummary(r.valorantMatch)
    }

    if (!!r.wowChallenge) {
        cleanWowChallengeFromJson(r.wowChallenge)
    }

    if (!!r.wowEncounter) {
        cleanWowEncounterFromJson(r.wowEncounter)
    }

    if (!!r.wowArena) {
        cleanWowArenaFromJson(r.wowArena)
    }

    if (!!r.csgoMatch) {
        cleanCsgoPlayerMatchSummaryFromJson(r.csgoMatch)
    }

    return r
}