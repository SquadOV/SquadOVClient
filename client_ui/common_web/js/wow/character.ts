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

export const MAGE_CLASS_ID = 8
export const PALLY_CLASS_ID = 2
export const WARRIOR_CLASS_ID = 1
export const DRUID_CLASS_ID = 11
export const DK_CLASS_ID = 6
export const HUNTER_CLASS_ID = 3
export const PRIEST_CLASS_ID = 5
export const ROGUE_CLASS_ID = 4
export const SHAMAN_CLASS_ID = 7
export const WARLOCK_CLASS_ID = 9
export const MONK_CLASS_ID = 10
export const DH_CLASS_ID = 12

export function specIdToClassId(specId: number): number {
    switch (specId) {
        // MAGE
        case 62:
        case 63:
        case 64:
        case 1449:
            return MAGE_CLASS_ID
        // PALLY
        case 65:
        case 66:
        case 70:
        case 1451:
            return PALLY_CLASS_ID
        // WARRIOR
        case 71:
        case 72:
        case 73:
        case 1446:
            return WARRIOR_CLASS_ID
        // DRUID
        case 102:
        case 103:
        case 104:
        case 105:
        case 1447:
            return DRUID_CLASS_ID
        // DEATHKNIGHT
        case 250:
        case 251:
        case 252:
        case 1455:
            return DK_CLASS_ID
        // HUNTER
        case 253:
        case 254:
        case 255:
        case 1448:
            return HUNTER_CLASS_ID
        // PRIEST
        case 256:
        case 257:
        case 258:
        case 1452:
            return PRIEST_CLASS_ID
        // ROGUE
        case 259:
        case 260:
        case 261:
        case 1453:
            return ROGUE_CLASS_ID
        // SHAMAN
        case 262:
        case 263:
        case 264:
        case 1444:
            return SHAMAN_CLASS_ID
        // WARLOCK
        case 265:
        case 266:
        case 267:
        case 1454:
            return WARLOCK_CLASS_ID
        // MONK
        case 268:
        case 269:
        case 270:
        case 1450:
            return MONK_CLASS_ID
        // DH
        case 577:
        case 581:
        case 1456:
            return DH_CLASS_ID
        default:
            return -1
    }
}