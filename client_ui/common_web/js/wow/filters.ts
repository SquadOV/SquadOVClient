import { WowInstanceType } from "@client/js/wow/matches";

export interface WowMatchFilters {
    hasVod: boolean
    encounters: number[]
    raids: number[]
    dungeons: number[]
    arenas: number[]
    brackets: string[]
    ratingLow: number | undefined
    ratingHigh: number | undefined
    friendlyComposition: string[]
    enemyComposition: string[]
    povSpec: number[]
    encounterDifficulties: number[]
    keystoneLow: number | undefined
    keystoneHigh: number | undefined
    isWinner: boolean | null
    instanceTypes: WowInstanceType[]
    enabled: boolean
}

export function createEmptyWowMatchFilters(): WowMatchFilters {
    return {
        hasVod: true,
        encounters: [],
        raids: [],
        dungeons: [],
        arenas: [],
        brackets: [],
        ratingLow: undefined,
        ratingHigh: undefined,
        friendlyComposition: [],
        enemyComposition: [],
        povSpec: [],
        encounterDifficulties: [],
        keystoneLow: undefined,
        keystoneHigh: undefined,
        isWinner: null,
        instanceTypes: [],
        enabled: true,
    }
}