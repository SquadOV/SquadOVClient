export interface AimlabMatchFilters {
    tasks: string[]
    hasVod: boolean
}

export function createEmptyAimlabMatchFilters(): AimlabMatchFilters {
    return {
        tasks: [],
        hasVod: true,
    }
}