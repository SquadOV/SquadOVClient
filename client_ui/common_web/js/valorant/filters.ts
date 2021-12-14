export enum ValorantMatchFilterEvents {
    DoubleKill,
    TripleKill,
    QuadraKill,
    PentaKill,
}

export const ALL_VALORANT_MATCH_FILTER_EVENTS = [
    ValorantMatchFilterEvents.DoubleKill,
    ValorantMatchFilterEvents.TripleKill,
    ValorantMatchFilterEvents.QuadraKill,
    ValorantMatchFilterEvents.PentaKill,
]

export function filterEventToName(e: ValorantMatchFilterEvents): string {
    switch (e) {
        case ValorantMatchFilterEvents.DoubleKill:
            return '2K'
        case ValorantMatchFilterEvents.TripleKill:
            return '3K'
        case ValorantMatchFilterEvents.QuadraKill:
            return '4K'
        case ValorantMatchFilterEvents.PentaKill:
            return '5K+'
    }
    return 'Unknown'
}

export interface ValorantMatchFilters {
    hasVod: boolean
    isRanked: boolean
    modes: string[]
    maps: string[]
    agentPovs: string[]
    isWinner: boolean | null
    rankLow: number | undefined
    rankHigh: number | undefined
    povEvents: ValorantMatchFilterEvents[]
    friendlyComposition: string[][]
    enemyComposition: string[][]
}

export function createEmptyValorantMatchFilters(): ValorantMatchFilters {
    return {
        hasVod: true,
        isRanked: false,
        modes: [],
        maps: [],
        agentPovs: [],
        isWinner: null,
        rankLow: undefined,
        rankHigh: undefined,
        povEvents: [],
        friendlyComposition: [],
        enemyComposition: [],
    }
}

export function migrateValorantMatchFilters(f: ValorantMatchFilters) {
    if (f.agentPovs === undefined) {
        f.agentPovs = []
    }

    if (f.isWinner === undefined) {
        f.isWinner = null
    }

    if (f.povEvents === undefined) {
        f.povEvents = []
    }

    if (f.friendlyComposition === undefined) {
        f.friendlyComposition = []
    }

    if (f.enemyComposition === undefined) {
        f.enemyComposition = []
    }
}