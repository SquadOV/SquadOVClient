export interface WowCharacter {
    guid: string
    name: string
    ilvl: number
    specId: number
    team: number
}

export interface WowClassSpecData {
    id: string
    name: string
    class: string
}

export interface WowClassData {
    id: string
    name: string
}

export interface WoWCharacterUserAssociation {
    userId: number
    guid: string
}