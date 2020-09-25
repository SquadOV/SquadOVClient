import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import { getValorantContent } from '@client/js/valorant/valorant_content'

export interface ValorantPlayerMatchSummary {
    matchId : string
    matchTime : Date
    map : string
    provisioningFlowId : string
    gameMode : string
    win : boolean
    patchId : string
    hasVod: boolean
    myTeamScore: number
    otherTeamScore: number
    isRanked: boolean

    agentId : string
    stats : ValorantPlayerStatsSummary
    csRank: number
}

export function getGameMode(s : ValorantPlayerMatchSummary) {
    let cnt = getValorantContent(s.patchId)
    let mode = cnt.gameModeToName(s.gameMode)
    if (mode == "Standard") {
        if (s.isRanked) {
            return "Competitive"
        } else {
            return "Unrated"
        }
    } else {
        return mode
    }
}

export function getIsCustom(s : ValorantPlayerMatchSummary) : boolean {
    return s.provisioningFlowId === 'CustomGame'
}