export interface TftMatchFilters {
    hasVod: boolean
}

export function createEmptyTftMatchFilters(): TftMatchFilters {
    return {
        hasVod: false,
    }
}