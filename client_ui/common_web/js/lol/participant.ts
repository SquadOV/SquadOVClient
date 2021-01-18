export interface LolParticipantIdentity {
    participantId: number
    player: LolPlayer | null
}

export interface LolPlayer {
    accountId: number
    currentAccountId: number
    currentPlatformId: number
    summonerName: string
    summonerId: string | null
    platformId: string
}

export interface LolParticipant {
    participantId: number
    championId: number
    teamId: number
    spell1Id: number
    spell2Id: number
    stats: LolParticipantStats
    timeline: LolParticipantTimeline
}

export interface WrappedLolParticipant {
    participant: LolParticipant
    identity: LolParticipantIdentity | undefined
}

export interface LolParticipantStats {
    // Player Identity
    participantId: number
    champLevel: number
    win: boolean
    // KDA
    kills: number
    deaths: number
    assists: number
    // Items
    item0: number
    item1: number
    item2: number
    item3: number
    item4: number
    item5: number
    item6: number
    // Notable kills
    doubleKills: number
    tripleKills: number
    quadraKills: number
    pentaKills: number
    firstBloodKill: boolean
    // Econ Stats
    goldEarned: number
    goldSpent: number
    // Neutral Stats
    neutralMinionsKilledTeamJungle: number
    neutralMinionsKilledEnemyJungle: number
    wardsKilled: number
    wardsPlaced: number
    visionWardsBoughtInGame: number
    sightWardsBoughtInGame: number
    neutralMinionsKills: number
    totalMinionsKilled: number
    // Objective Stats
    damageDealtToObjectives: number
    inhibitorKills: number
    turretKills: number
    damageDealtToTurrets: number
    // Score
    totalPlayerScore: number
    totalScoreRank: number
    objectivePlayerScore: number
    combatPlayerScore: number
    visionScore: number
    // Damage Dealt to Champions
    totalDamageDealtToChampions: number
    physicalDamageDealtToChampions: number
    magicDamageDealtToChampions: number
    trueDamageDealtToChampions: number
    // Damage Dealt
    totalDamageDealt: number
    physicalDamageDealt: number
    magicDamageDealt: number 
    trueDamageDealt: number
    // Damage Taken
    totalDamageTaken: number 
    physicalDamageToken: number
    magicalDamageTaken: number
    trueDamageTaken: number
    // Other Combat  
    totalHeal: number
    damageSelfMitigated: number
}

export interface LolParticipantTimeline {
    participantId: number
    lane: string
}