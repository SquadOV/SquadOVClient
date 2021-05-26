export interface ValorantMatchFilters {
    hasVod: boolean
    isRanked: boolean,
    modes: string[]
    maps: string[]
}

export function createEmptyValorantMatchFilters(): ValorantMatchFilters {
    return {
        hasVod: false,
        isRanked: false,
        modes: [],
        maps: [],
    }
}