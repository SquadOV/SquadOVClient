import { LolParticipant } from '@client/js/lol/participant'
import { LolTeamStats } from '@client/js/lol/team'
import { LolMatchTimeline, LolMatchFrameStat, LolMatchFrame, getLolMatchNonEmptyTimelineFrames } from '@client/js/lol/timeline'

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
    totalDamageDealtToChampions: number
    totalMinionsKilled: number
    wardsPlaced: number
    lane: string
    win: boolean
}

export interface FullLolMatch {
    match: LolMatch
    timeline: LolMatchTimeline
    userIdToParticipantId: { [userId: number] : number | undefined }
    gameStartTime: Date | null
}

export interface LolMatch {
    info: {
        gameId: number
        queueId: number
        gameType: string
        gameDuration: number
        gameCreation: Date
        gameVersion: string
        mapId: number
        gameMode: string
        teams: LolTeamStats[]
        participants: LolParticipant[]
    }
}

export function cleanLolPlayerMatchSummaryFromJson(s: LolPlayerMatchSummary): LolPlayerMatchSummary {
    s.gameCreation = new Date(s.gameCreation)
    return s
}

export function cleanFullLolMatch(s: FullLolMatch): FullLolMatch {
    s.match.info.gameCreation = new Date(s.match.info.gameCreation)
    if (!!s.gameStartTime) {
        s.gameStartTime = new Date(s.gameStartTime)
    }
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

export function getTeamIdFromParticipantId(match: LolMatch, id: number): number | undefined {
    return match.info.participants.find((ele: LolParticipant) => ele.participantId === id)?.teamId
}

export function extractSameTeamPlayersFromParticipantId(match: LolMatch, id: number): LolParticipant[] | undefined {
    let teamId = getTeamIdFromParticipantId(match, id)
    if (!teamId) {
        return undefined
    }
    return extractSameTeamPlayersFromTeamId(match, teamId)
}

export function extractEnemyTeamPlayersFromParticipantId(match: LolMatch, id: number): LolParticipant[] | undefined {
    let teamId = getTeamIdFromParticipantId(match, id)
    if (!teamId) {
        return undefined
    }
    return extractSameTeamPlayersFromTeamId(match, (teamId === 100) ? 200 : 100)
}

export function extractSameTeamPlayersFromTeamId(match: LolMatch, id: number): LolParticipant[] {
    let players = match.info.participants.filter((ele: LolParticipant) => {
        return ele.teamId === id
    })

    players.sort((a: LolParticipant, b: LolParticipant) => {
        return b.kills - a.kills
    })

    return players
}

export function getPlayerFromParticipantId(match: LolMatch, id: number): LolParticipant | undefined {
    let player = match.info.participants.find((ele: LolParticipant) => {
        return ele.participantId === id
    })

    if (!player) {
        return undefined
    }

    return player
}

export function extractLolFrameStatsFromTimelineForPlayer(match: LolMatch, timeline: LolMatchTimeline, stat: LolMatchFrameStat, id: number): number[] {
    let frameStats = getLolMatchNonEmptyTimelineFrames(timeline).map((ele: LolMatchFrame) => {
        let frame = ele.participantFrames[`${id}`]!
        switch (stat) {
            case LolMatchFrameStat.EGold:
                return frame.totalGold
            case LolMatchFrameStat.ELevel:
                return frame.level
            case LolMatchFrameStat.EMinions:
                return frame.minionsKilled
            case LolMatchFrameStat.EXp:
                return frame.xp
        }
    })

    let player = getPlayerFromParticipantId(match, id)
    let lastStatVal: number = frameStats[frameStats.length - 1]
    if (!!player) {
        switch (stat) {
            case LolMatchFrameStat.EGold:
                lastStatVal = player.goldEarned
            case LolMatchFrameStat.ELevel:
                lastStatVal = player.champLevel
            case LolMatchFrameStat.EMinions:
                lastStatVal = player.totalMinionsKilled
            case LolMatchFrameStat.EXp:
                lastStatVal = frameStats[frameStats.length - 1]
        }
    }

    return [
        ...frameStats,
        lastStatVal
    ]
}

export function extractLolFrameStatsFromTimelineForTeam(match: LolMatch, timeline: LolMatchTimeline, stat: LolMatchFrameStat, id: number): number[] {
    let players = extractSameTeamPlayersFromTeamId(match, id)
    let stats = players.map((p: LolParticipant) => extractLolFrameStatsFromTimelineForPlayer(match, timeline, stat, p.participantId))
    if (stats.length == 1) {
        return stats[0]
    } else if (stats.length == 0) {
        return []
    } else {
        return stats.slice(1).reduce((prev: number[], input: number[]) => {
            return prev.map((y: number, idx: number) => {
                return y + input[idx]
            })
        }, stats[0])
    }
}

export function getLolOpposingPlayer(match: LolMatch, timeline: LolMatchTimeline, id: number): number | undefined {
    let player = getPlayerFromParticipantId(match, id)
    if (!player) {
        return undefined
    }

    let enemies = extractEnemyTeamPlayersFromParticipantId(match, id)
    if (!enemies) {
        return undefined
    }

    let lane = player.lane
    return enemies.find((ele: LolParticipant) => ele.lane === lane)?.participantId
}