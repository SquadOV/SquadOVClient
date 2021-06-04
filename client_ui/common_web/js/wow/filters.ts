export interface WowMatchFilters {
    hasVod: boolean
    encounters: number[]
    raids: number[]
    dungeons: number[]
    arenas: number[]
    brackets: string[]
}

export function createEmptyWowMatchFilters(): WowMatchFilters {
    return {
        hasVod: false,
        encounters: [],
        raids: [],
        dungeons: [],
        arenas: [],
        brackets: [],
    }
}