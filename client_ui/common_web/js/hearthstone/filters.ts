export interface HearthstoneMatchFilters {
    hasVod: boolean
}

export function createEmptyHearthstoneMatchFilters(): HearthstoneMatchFilters {
    return {
        hasVod: true,
    }
}