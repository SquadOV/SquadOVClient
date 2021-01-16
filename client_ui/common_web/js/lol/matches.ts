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
    currentParticipantId: number
    participants: LolMiniParticipantStats[]
    hasVod: boolean
}

export interface LolMiniParticipantStats {
    participantId: number
    championId: number
    summonerName: string | null
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

export function cleanLolPlayerMatchSummaryFromJson(s: LolPlayerMatchSummary): LolPlayerMatchSummary {
    s.gameCreation = new Date(s.gameCreation)
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