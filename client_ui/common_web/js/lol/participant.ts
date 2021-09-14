export interface LolParticipant {
    assists: number
    baronKills: number
    bountyLevel: number
    champExperience: number
    champLevel: number
    championId: number
    championName: string
    championTransform: number
    consumablesPurchased: number
    damageDealtToBuildings: number
    damageDealtToObjectives: number
    damageDealtToTurrets: number
    damageSelfMitigated: number
    deaths: number
    detectorWardsPlaced: number
    doubleKills: number
    dragonKills: number
    firstBloodAssist: boolean
    firstBloodKill: boolean
    firstTowerAssist: boolean
    firstTowerKill: boolean
    gameEndedInEarlySurrender: boolean
    gameEndedInSurrender: boolean
    goldEarned: number
    goldSpent: number
    individualPosition: string
    inhibitorKills: number
    inhibitorTakedowns: number
    inhibitorsLost: number
    item0: number
    item1: number
    item2: number
    item3: number
    item4: number
    item5: number
    item6: number
    itemsPurchased: number
    killingSprees: number
    kills: number
    lane: string
    largestCriticalStrike: number
    largestKillingSpree: number
    largestMultiKill: number
    longestTimeSpentLiving: number
    magicDamageDealt: number
    magicDamageDealtToChampions: number
    magicDamageTaken: number
    neutralMinionsKilled: number
    nexusKills: number
    nexusTakedowns: number
    nexusLost: number
    objectivesStolen: number
    objectivesStolenAssists: number
    participantId: number
    pentaKills: number
    physicalDamageDealt: number
    physicalDamageDealtToChampions: number
    physicalDamageTaken: number
    profileIcon: number
    puuid: string
    quadraKills: number
    riotIdName: string
    riotIdTagline: string
    role: string
    sightWardsBoughtInGame: number
    spell1Casts: number
    spell2Casts: number
    spell3Casts: number
    spell4Casts: number
    summoner1Casts: number
    summoner1Id: number
    summoner2Casts: number
    summoner2Id: number
    summonerId: string
    summonerLevel: number
    summonerName: string
    teamEarlySurrendered: boolean
    teamId: number
    teamPosition: string
    timeCcingOthers: number
    timePlayed: number
    totalDamageDealt: number
    totalDamageDealtToChampions: number
    totalDamageShieldedOnTeammates: number
    totalDamageTaken: number
    totalHeal: number
    totalHealsOnTeammates: number
    totalMinionsKilled: number
    totalTimeCcDealt: number
    totalTimeSpentDead: number
    totalUnitsHealed: number
    tripleKills: number
    trueDamageDealt: number
    trueDamageDealtToChampions: number
    trueDamageTaken: number
    turretKills: number
    turretTakedowns: number
    turretsLost: number
    unrealKills: number
    visionScore: number
    visionWardsBoughtInGame: number
    wardsKilled: number
    wardsPlaced: number
    win: boolean
}

export enum ELolDisplayStat {
    // KDA
    KDA,
    DK,
    TK,
    QK,
    PK,
    FirstBlood,
    // Economy
    GoldEarned,
    GoldSpent,
    // Neutral
    NeutralsKilled,
    MinionsKilled,
    // Vision
    WardsKilled,
    WardsPlaced,
    VisionWardsBought,
    SightWardsBought,
    // Objectives
    DamageToObjectives,
    InhibitorKills,
    TowerKills,
    DamageToTurrets,
    // Score
    VisionScore,
    // Damage Dealt
    TotalDamageDealt,
    PhysicalDealt,
    MagicDealt,
    TrueDealt,
    TotalDamageDealtToChamps,
    PhysicalDealtToChamps,
    MagicDealtToChamps,
    TrueDealtToChamps,
    // Damage Taken
    TotalDamageTaken,
    PhysicalTaken,
    MagicTaken,
    TrueTaken,
    // Other Damage
    TotalHeal,
    DamageSelfMitigated,
}

export let allLolDisplayStats: ELolDisplayStat[] = [
    ELolDisplayStat.KDA,
    ELolDisplayStat.DK,
    ELolDisplayStat.TK,
    ELolDisplayStat.QK,
    ELolDisplayStat.PK,
    ELolDisplayStat.FirstBlood,
    ELolDisplayStat.VisionScore,
    ELolDisplayStat.GoldEarned,
    ELolDisplayStat.GoldSpent,
    ELolDisplayStat.NeutralsKilled,
    ELolDisplayStat.MinionsKilled,
    ELolDisplayStat.WardsKilled,
    ELolDisplayStat.WardsPlaced,
    ELolDisplayStat.VisionWardsBought,
    ELolDisplayStat.SightWardsBought,
    ELolDisplayStat.DamageToObjectives,
    ELolDisplayStat.InhibitorKills,
    ELolDisplayStat.TowerKills,
    ELolDisplayStat.DamageToTurrets,
    ELolDisplayStat.TotalDamageDealt,
    ELolDisplayStat.PhysicalDealt,
    ELolDisplayStat.MagicDealt,
    ELolDisplayStat.TrueDealt,
    ELolDisplayStat.TotalDamageDealtToChamps,
    ELolDisplayStat.PhysicalDealtToChamps,
    ELolDisplayStat.MagicDealtToChamps,
    ELolDisplayStat.TrueDealtToChamps,
    ELolDisplayStat.TotalDamageTaken,
    ELolDisplayStat.PhysicalTaken,
    ELolDisplayStat.MagicTaken,
    ELolDisplayStat.TrueTaken,
    ELolDisplayStat.TotalHeal,
    ELolDisplayStat.DamageSelfMitigated,
]

export enum ELolStatCategory {
    Kda,
    Economy,
    Neutrals,
    Vision,
    Objectives,
    Score,
    DamageDealt,
    DamageTaken,
    OtherDamage
}

export let allLolStatCategories: ELolStatCategory[] = [
    ELolStatCategory.Kda,
    ELolStatCategory.Economy,
    ELolStatCategory.Neutrals,
    ELolStatCategory.Vision,
    ELolStatCategory.Objectives,
    ELolStatCategory.Score,
    ELolStatCategory.DamageDealt,
    ELolStatCategory.DamageTaken,
    ELolStatCategory.OtherDamage
]

export function getLolDisplayStatsForCategory(c: ELolStatCategory): ELolDisplayStat[] {
    return allLolDisplayStats.filter((ele: ELolDisplayStat) => {
        return lolDisplayStatToCategory(ele) === c
    })
}

export function lolStatCategoryToName(c: ELolStatCategory) : string {
    switch (c) {
        case ELolStatCategory.Kda:
            return 'KDA'
        case ELolStatCategory.Economy:
            return 'Economy'
        case ELolStatCategory.Neutrals:
            return 'Neutrals'
        case ELolStatCategory.Vision:
            return 'Vision'
        case ELolStatCategory.Objectives:
            return 'Objectives'
        case ELolStatCategory.Score:
            return 'Score'
        case ELolStatCategory.DamageDealt:
            return 'Damage Dealt'
        case ELolStatCategory.DamageTaken:
            return 'Damage Taken'
        case ELolStatCategory.OtherDamage:
            return 'Other Damage'
    }
}

export function lolDisplayStatToCategory(s: ELolDisplayStat): ELolStatCategory {
    switch (s) {
        // KDA
        case ELolDisplayStat.KDA:
        case ELolDisplayStat.DK:
        case ELolDisplayStat.TK:
        case ELolDisplayStat.QK:
        case ELolDisplayStat.PK:
        case ELolDisplayStat.FirstBlood:
            return ELolStatCategory.Kda
        // Economy
        case ELolDisplayStat.GoldEarned:
        case ELolDisplayStat.GoldSpent:
            return ELolStatCategory.Economy
        // Neutral
        case ELolDisplayStat.NeutralsKilled:
        case ELolDisplayStat.MinionsKilled:
            return ELolStatCategory.Neutrals
        // Vision
        case ELolDisplayStat.WardsKilled:
        case ELolDisplayStat.WardsPlaced:
        case ELolDisplayStat.VisionWardsBought:
        case ELolDisplayStat.SightWardsBought:
            return ELolStatCategory.Vision
        // Objectives
        case ELolDisplayStat.DamageToObjectives:
        case ELolDisplayStat.InhibitorKills:
        case ELolDisplayStat.TowerKills:
        case ELolDisplayStat.DamageToTurrets:
            return ELolStatCategory.Objectives
        // Score
        case ELolDisplayStat.VisionScore:
            return ELolStatCategory.Score
        // Damage Dealt
        case ELolDisplayStat.TotalDamageDealt:
        case ELolDisplayStat.PhysicalDealt:
        case ELolDisplayStat.MagicDealt:
        case ELolDisplayStat.TrueDealt:
        case ELolDisplayStat.TotalDamageDealtToChamps:
        case ELolDisplayStat.PhysicalDealtToChamps:
        case ELolDisplayStat.MagicDealtToChamps:
        case ELolDisplayStat.TrueDealtToChamps:
            return ELolStatCategory.DamageDealt
        // Damage Taken
        case ELolDisplayStat.TotalDamageTaken:
        case ELolDisplayStat.PhysicalTaken:
        case ELolDisplayStat.MagicTaken:
        case ELolDisplayStat.TrueTaken:
            return ELolStatCategory.DamageTaken
        // Other Damage
        case ELolDisplayStat.TotalHeal:
        case ELolDisplayStat.DamageSelfMitigated:
            return ELolStatCategory.OtherDamage
    }
}

export function lolDisplayStatToName(s: ELolDisplayStat): string {
    switch (s) {
        // KDA
        case ELolDisplayStat.KDA:
            return 'KDA'
        case ELolDisplayStat.DK:
            return 'Double Kills'
        case ELolDisplayStat.TK:
            return 'Triple Kills'
        case ELolDisplayStat.QK:
            return 'Quadra Kills'
        case ELolDisplayStat.PK:
            return 'Penta Kills'
        case ELolDisplayStat.FirstBlood:
            return 'First Blood'
        // Economy
        case ELolDisplayStat.GoldEarned:
            return 'Gold Earned'
        case ELolDisplayStat.GoldSpent:
            return 'Gold Spent'
        // Neutral
        case ELolDisplayStat.NeutralsKilled:
            return 'Neutrals Killed'
        case ELolDisplayStat.MinionsKilled:
            return 'Minions Killed'
        // Vision
        case ELolDisplayStat.WardsKilled:
            return 'Wards Killed'
        case ELolDisplayStat.WardsPlaced:
            return 'Wards Placed'
        case ELolDisplayStat.VisionWardsBought:
            return 'Vision Wards Bought'
        case ELolDisplayStat.SightWardsBought:
            return 'Sight Wards Bought'
        // Objectives
        case ELolDisplayStat.DamageToObjectives:
            return 'Damage To Objectives'
        case ELolDisplayStat.InhibitorKills:
            return 'Inhibitor Kills'
        case ELolDisplayStat.TowerKills:
            return 'Tower Kills'
        case ELolDisplayStat.DamageToTurrets:
            return 'Damage To Turrets'
        // Score
        case ELolDisplayStat.VisionScore:
            return 'Vision Score'
        // Damage Dealt
        case ELolDisplayStat.TotalDamageDealt:
        case ELolDisplayStat.TotalDamageTaken:
            return 'Total Damage'
        case ELolDisplayStat.PhysicalDealt:
        case ELolDisplayStat.PhysicalTaken:
            return 'Physical Damage'
        case ELolDisplayStat.MagicDealt:
        case ELolDisplayStat.MagicTaken:
            return 'Magic Damage'
        case ELolDisplayStat.TrueDealt:
        case ELolDisplayStat.TrueTaken:
            return 'True Damage'
        case ELolDisplayStat.TotalDamageDealtToChamps:
            return 'Total Damage (Champions)'
        case ELolDisplayStat.PhysicalDealtToChamps:
            return 'Physical Damage (Champions)'
        case ELolDisplayStat.MagicDealtToChamps:
            return 'Magic Damage (Champions)'
        case ELolDisplayStat.TrueDealtToChamps:
            return 'True Damage (Champions)'
        // Other Damage
        case ELolDisplayStat.TotalHeal:
            return 'Heals'
        case ELolDisplayStat.DamageSelfMitigated:
            return 'Damage Self-Mitigated'
    }
}

export function extractLolDisplayStatFromParticipant(p: LolParticipant, s: ELolDisplayStat): any {
    switch (s) {
        // KDA
        case ELolDisplayStat.KDA:
            return `${p.kills} / ${p.deaths} / ${p.assists}`
        case ELolDisplayStat.DK:
            return p.doubleKills
        case ELolDisplayStat.TK:
            return p.tripleKills
        case ELolDisplayStat.QK:
            return p.quadraKills
        case ELolDisplayStat.PK:
            return p.pentaKills
        case ELolDisplayStat.FirstBlood:
            return p.firstBloodKill
        // Economy
        case ELolDisplayStat.GoldEarned:
            return p.goldEarned
        case ELolDisplayStat.GoldSpent:
            return p.goldSpent
        // Neutral
        case ELolDisplayStat.NeutralsKilled:
            return p.neutralMinionsKilled
        case ELolDisplayStat.MinionsKilled:
            return p.totalMinionsKilled
        // Vision
        case ELolDisplayStat.WardsKilled:
            return p.wardsKilled
        case ELolDisplayStat.WardsPlaced:
            return p.wardsPlaced
        case ELolDisplayStat.VisionWardsBought:
            return p.visionWardsBoughtInGame
        case ELolDisplayStat.SightWardsBought:
            return p.sightWardsBoughtInGame
        // Objectives
        case ELolDisplayStat.DamageToObjectives:
            return p.damageDealtToObjectives
        case ELolDisplayStat.InhibitorKills:
            return p.inhibitorKills
        case ELolDisplayStat.TowerKills:
            return p.turretKills
        case ELolDisplayStat.DamageToTurrets:
            return p.damageDealtToTurrets
        // Score
        case ELolDisplayStat.VisionScore:
            return p.visionScore
        // Damage Dealt
        case ELolDisplayStat.TotalDamageDealt:
            return p.totalDamageDealt
        case ELolDisplayStat.TotalDamageTaken:
            return p.totalDamageTaken
        case ELolDisplayStat.PhysicalDealt:
            return p.physicalDamageDealt
        case ELolDisplayStat.PhysicalTaken:
            return p.physicalDamageTaken
        case ELolDisplayStat.MagicDealt:
            return p.magicDamageDealt
        case ELolDisplayStat.MagicTaken:
            return p.magicDamageTaken
        case ELolDisplayStat.TrueDealt:
            return p.trueDamageDealt
        case ELolDisplayStat.TrueTaken:
            return p.trueDamageTaken
        case ELolDisplayStat.TotalDamageDealtToChamps:
            return p.totalDamageDealtToChampions
        case ELolDisplayStat.PhysicalDealtToChamps:
            return p.physicalDamageDealtToChampions
        case ELolDisplayStat.MagicDealtToChamps:
            return p.magicDamageDealtToChampions
        case ELolDisplayStat.TrueDealtToChamps:
            return p.trueDamageDealtToChampions
        // Other Damage
        case ELolDisplayStat.TotalHeal:
            return p.totalHeal
        case ELolDisplayStat.DamageSelfMitigated:
            return p.damageSelfMitigated
    }
}