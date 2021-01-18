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
    physicalDamageTaken: number
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
    JungleTeam,
    JungleEnemy,
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
    TotalScore,
    ObjectiveScore,
    CombatScore,
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
    ELolDisplayStat.TotalScore,
    ELolDisplayStat.ObjectiveScore,
    ELolDisplayStat.CombatScore,
    ELolDisplayStat.VisionScore,
    ELolDisplayStat.GoldEarned,
    ELolDisplayStat.GoldSpent,
    ELolDisplayStat.JungleTeam,
    ELolDisplayStat.JungleEnemy,
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
        case ELolDisplayStat.JungleTeam:
        case ELolDisplayStat.JungleEnemy:
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
        case ELolDisplayStat.TotalScore:
        case ELolDisplayStat.ObjectiveScore:
        case ELolDisplayStat.CombatScore:
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
        case ELolDisplayStat.JungleTeam:
            return 'Killed in Team Jungle'
        case ELolDisplayStat.JungleEnemy:
            return 'Killed in Enemy Jungle'
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
        case ELolDisplayStat.TotalScore:
            return 'Total Score'
        case ELolDisplayStat.ObjectiveScore:
            return 'Objective Score'
        case ELolDisplayStat.CombatScore:
            return 'Combat Score'
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

export function extractLolDisplayStatFromParticipant(p: LolParticipantStats, s: ELolDisplayStat): any {
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
        case ELolDisplayStat.JungleTeam:
            return p.neutralMinionsKilledTeamJungle
        case ELolDisplayStat.JungleEnemy:
            return p.neutralMinionsKilledEnemyJungle
        case ELolDisplayStat.NeutralsKilled:
            return p.neutralMinionsKills
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
        case ELolDisplayStat.TotalScore:
            return p.totalPlayerScore
        case ELolDisplayStat.ObjectiveScore:
            return p.objectivePlayerScore
        case ELolDisplayStat.CombatScore:
            return p.combatPlayerScore
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
            return p.magicalDamageTaken
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