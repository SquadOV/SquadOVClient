export interface WowDifficultyData {
    id: string
    name: string
}

export const ALL_WOW_ENCOUNTER_DIFFICULTIES: number[] = [
    14,
    15,
    16,
    17
]

export function encounterDifficultyToName(diff: number): string {
    switch (diff) {
        case 14:
            return 'Normal'
        case 15:
            return 'Heroic'
        case 16:
            return 'Mythic'
        case 17:
            return 'LFR'
        default:
            return 'Unknown'
    }
}