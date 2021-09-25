export interface CsgoMatchFilters {
    modes: string[]
    maps: string[]
    hasVod: boolean
    hasDemo: boolean
}

export function createEmptyCsgoMatchFilters(): CsgoMatchFilters {
    return {
        modes: [],
        maps: [],
        hasVod: true,
        hasDemo: true,
    }
}