export interface ValorantMatchFilters {
    hasVod: boolean
    isRanked: boolean,
    modes: string[]
    maps: string[]
}

export function createEmptyValorantMatchFilters(): ValorantMatchFilters {
    return {
        hasVod: true,
        isRanked: false,
        modes: [],
        maps: [],
    }
}