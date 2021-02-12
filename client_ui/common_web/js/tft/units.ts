export interface TftUnit {
    items: number[],
    characterId: string | null,
    chosen: string | null,
    name: string,
    rarity: number,
    tier: number
}

export interface TftUnitData {
    name: string
    cost: number
    traits: string[]
}