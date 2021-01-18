import { LolParticipantIdentity, LolParticipant } from '@client/js/lol/participant'
import { LolTeamStats } from '@client/js/lol/team'
import { LolMatchTimeline } from '@client/js/lol/timeline'

export interface LolPlayerMatchSummary {
    matchUuid: string
    gameCreation: Date
    gameDuration: number
    gameType: string
    gameVersion: string
    queueId: number
    seasonId: number
    mapId: number
    gameMode: string
    currentParticipantId: number | undefined | null
    participants: LolMiniParticipantStats[]
    hasVod: boolean
}

export interface LolMiniParticipantStats {
    participantId: number
    championId: number
    summonerName: string | null | undefined
    teamId: number
    kills: number
    deaths: number
    assists: number
    totalDamageDealt: number
    totalMinionsKilled: number
    wardsPlaced: number
    lane: string
    win: boolean
}

export interface FullLolMatch {
    match: LolMatch
    timeline: LolMatchTimeline
    userIdToParticipantId: { [userId: number] : number | undefined }
}

export interface LolMatch {
    gameId: number
    queueId: number
    gameType: string
    gameDuration: number
    platformId: string
    gameCreation: Date
    seasonId: number
    gameVersion: string
    mapId: number
    gameMode: string
    participantIdentities: LolParticipantIdentity[]
    teams: LolTeamStats[]
    participants: LolParticipant[]
}

export function cleanLolPlayerMatchSummaryFromJson(s: LolPlayerMatchSummary): LolPlayerMatchSummary {
    s.gameCreation = new Date(s.gameCreation)
    return s
}

export function cleanFullLolMatch(s: FullLolMatch): FullLolMatch {
    s.match.gameCreation = new Date(s.match.gameCreation)
    return s
}

export function getLolGameMode(queueId: number, gameMode: string): string {
    switch (gameMode) {
        case 'CLASSIC':
            switch (queueId) {
                case 400:
                    return 'Draft Pick'
                case 420:
                    return 'Ranked (Solo)'
                case 430:
                    return 'Blind Pick'
                case 440:
                    return 'Ranked (Flex)'
            }
        case 'ODIN':
            return 'Dominion'
        case 'ARAM':
            return 'ARAM'
        case 'TUTORIAL':
            return 'Tutorial'
        case 'URF':
            return 'URF'
        case 'DOOMBOTSTEEMO':
            return 'Doom Bat'
        case 'ONEFORALL':
            return 'One for All'
        case 'ASCENSION':
            return 'Ascension'
        case 'FIRSTBLOOD':
            return 'Snowdown Showdown'
        case 'KINGPORO':
            return 'Legend of the Poro King'
        case 'SIEGE':
            return 'Nexus Siege'
        case 'ASSASSINATE':
            return 'Blood Hunt Assassin'
        case 'ARSR':
            return 'ARSR'
        case 'DARKSTAR':
            return 'Dark Star: Singularity'
        case 'STARGUARDIAN':
            return 'Star Guardian Invasion'
        case 'PROJECT':
            return 'PROJECT: Hunters'
        case 'GAMEMODEX':
            return 'Nexus Blitz'
        case 'ODYSSEY':
            return 'Odyssey: Extraction'
        case 'NEXUSBLITZ':
            return 'Nexus Blitz'
    }
    return 'Unknown'
}