export interface WowMatchFilters {
    hasVod: boolean
    encounters: number[]
    raids: number[]
    dungeons: number[]
    arenas: number[]
    brackets: string[]
    isWinner: boolean | null
}

export function createEmptyWowMatchFilters(): WowMatchFilters {
    return {
        hasVod: true,
        encounters: [],
        raids: [],
        dungeons: [],
        arenas: [],
        brackets: [],
        isWinner: null,
    }
}