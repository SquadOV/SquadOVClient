import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'

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

    agentId : string
    stats : ValorantPlayerStatsSummary
    csRank: number
}