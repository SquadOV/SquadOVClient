import { SquadOvGames } from '@client/js/squadov/game'
import { VodManifest } from '@client/js/squadov/vod'
import { AimlabTaskData, cleanAimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { LolPlayerMatchSummary, cleanLolPlayerMatchSummaryFromJson } from '@client/js/lol/matches'
import { TftPlayerMatchSummary, cleanTftPlayerMatchSummaryFromJson } from '@client/js/tft/matches'
import { WowChallenge, WowEncounter, WowArena, cleanWowChallengeFromJson, cleanWowEncounterFromJson, cleanWowArenaFromJson } from '@client/js/wow/matches'
import { ValorantPlayerMatchSummary, cleanValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'

export interface BaseRecentMatch {
    matchUuid: string
    tm: Date
    game: SquadOvGames
    vod: VodManifest
    username: string
    userId: number
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
}

export function checkRecentMatchValidity(r: RecentMatch): boolean {
    switch (r.base.game) {
        case SquadOvGames.AimLab:
            return !!r.aimlabTask
        case SquadOvGames.WorldOfWarcraft:
            return !!r.wowChallenge || !!r.wowEncounter || !!r.wowArena
        case SquadOvGames.TeamfightTactics:
            return !!r.tftMatch
        case SquadOvGames.LeagueOfLegends:
            return !!r.lolMatch
        case SquadOvGames.Valorant:
            return !!r.valorantMatch
        case SquadOvGames.Hearthstone:
            return true
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

    return r
}