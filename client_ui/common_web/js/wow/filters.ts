export interface WowMatchFilters {
    hasVod: boolean
    encounters: number[]
    raids: number[]
    dungeons: number[]
    arenas: number[]
}

export function createEmptyWowMatchFilters(): WowMatchFilters {
    return {
        hasVod: false,
        encounters: [],
        raids: [],
        dungeons: [],
        arenas: [],
    }
}