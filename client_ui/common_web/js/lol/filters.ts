export interface LolMatchFilters {
    maps: number[]
    modes: string[]
    hasVod: boolean
}

export function createEmptyLolMatchFilters(): LolMatchFilters {
    return {
        maps: [],
        modes: [],
        hasVod: true,
    }
}