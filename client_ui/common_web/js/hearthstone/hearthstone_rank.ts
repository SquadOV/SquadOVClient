export enum HearthstoneRank {
    Bronze,
    Silver,
    Gold,
    Platinum,
    Diamond,
    Legend
}

export function earnedStarsToHearthstoneRank(stars: number) : HearthstoneRank {
    if (stars > 50) {
        return HearthstoneRank.Legend
    } else if (stars > 40) {
        return HearthstoneRank.Diamond
    } else if (stars > 30) {
        return HearthstoneRank.Platinum
    } else if (stars > 20) {
        return HearthstoneRank.Gold
    } else if (stars > 10) {
        return HearthstoneRank.Silver
    } else {
        return HearthstoneRank.Bronze
    }
}

export function earnedStarsToHearthstoneSubrank(stars : number, legend: number) : number {
    if (stars > 50) {
        return legend
    } else {
        return 10 - (stars - 1) % 10
    }
}

export function hearthstoneRankToString(rank: HearthstoneRank) : string {
    switch (rank) {
        case HearthstoneRank.Bronze:
            return 'Bronze'
        case HearthstoneRank.Silver:
            return 'Silver'
        case HearthstoneRank.Gold:
            return 'Gold'
        case HearthstoneRank.Platinum:
            return 'Platinum'
        case HearthstoneRank.Diamond:
            return 'Diamond'
        case HearthstoneRank.Legend:
            return 'Legend'
    }
}