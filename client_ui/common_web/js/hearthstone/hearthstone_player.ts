export interface HearthstoneMedalInfo {
    leagueId: number
    earnedStars: number
    starLevel: number
    bestStarLevel: number
    winStreak: number
    legendIndex: number
}

export interface HearthstonePlayerMedalInfo {
    standard: HearthstoneMedalInfo
    wild: HearthstoneMedalInfo
}

export interface HearthstonePlayer {
    name: string
    local: boolean
    side: number
    cardBackId: number
    medalInfo: HearthstonePlayerMedalInfo
    arenaWins: number
    arenaLoss: number
    tavernBrawlWins: number
    tavernBrawlLoss: number
    battlegroundsRating: number | null
    duelsCasualRating: number | null
    duelsHeroicRating: number| null
}