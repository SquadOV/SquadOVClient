export interface WowCharacter {
    guid: string
    name: string
    ilvl: number
    specId: number
    team: number
    rating: number
    classId: number | null
}

export interface WowItem {
    itemId: number
    ilvl: number
}

export function wowItemSlotToName(slot: number): string {
    switch (slot) {
        case 0:
            return 'Unequippable'
        case 1:
            return 'Head'
        case 2:
            return 'Neck'
        case 3:
            return 'Shoulder'
        case 4:
            return 'Shirt'
        case 5: 
        case 20:
            return 'Chest'
        case 6:
            return 'Waist'
        case 7:
            return 'Legs'
        case 8:
            return 'Feet'
        case 9:
            return 'Wrist'
        case 10:
            return 'Hands'
        case 11:
            return 'Finger'
        case 12:
            return 'Trinket'
        case 13:
            return 'One-Hand'
        case 14:
        case 22:
        case 23:
            return 'Off Hand'
        case 15:
        case 26:
            return 'Ranged'
        case 16:
            return 'Back'
        case 17:
            return 'Two-Hand'
        case 18:
            return 'Bag'
        case 19:
            return 'Tabard'
        case 21:
            return 'Main Hand'
        case 24:
            return 'Ammo'
        case 25:
            return 'Thrown'
    }
    return 'Unknown Slot'
}

export interface WowCovenant {
    covenantId: number
    soulbindId: number
    soulbindTraits: number[]
    conduits: WowItem[]
}

export interface WowFullCharacter {
    items: WowItem[]
    covenant: WowCovenant | null
    talents: number[]
    pvpTalents: number[]
    rating: number
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

export const ALL_WOW_CLASSES = [
    MAGE_CLASS_ID,
    PALLY_CLASS_ID,
    WARRIOR_CLASS_ID,
    DRUID_CLASS_ID,
    DK_CLASS_ID,
    HUNTER_CLASS_ID,
    PRIEST_CLASS_ID,
    ROGUE_CLASS_ID,
    SHAMAN_CLASS_ID,
    WARLOCK_CLASS_ID,
    MONK_CLASS_ID,
    DH_CLASS_ID,
    -1,
]

export function classToName(classId: number): string {
    switch (classId) {
        case MAGE_CLASS_ID:
            return 'Mage'
        case PALLY_CLASS_ID:
            return 'Paladin'
        case WARRIOR_CLASS_ID:
            return 'Warrior'
        case DRUID_CLASS_ID:
            return 'Druid'
        case DK_CLASS_ID:
            return 'Death Knight'
        case HUNTER_CLASS_ID:
            return 'Hunter'
        case PRIEST_CLASS_ID:
            return 'Priest'
        case ROGUE_CLASS_ID:
            return 'Rogue'
        case SHAMAN_CLASS_ID:
            return 'Shaman'
        case WARLOCK_CLASS_ID:
            return 'Warlock'
        case MONK_CLASS_ID:
            return 'Monk'
        case DH_CLASS_ID:
            return 'Demon Hunter'
        default:
            return 'Unknown'
    }
}

export function classToSpecIds(classId: number): number[] {
    switch (classId) {
        case MAGE_CLASS_ID:
            return [62, 63, 64]
        case PALLY_CLASS_ID:
            return [65, 66, 70]
        case WARRIOR_CLASS_ID:
            return [71, 72, 73]
        case DRUID_CLASS_ID:
            return [102, 103, 104, 105]
        case DK_CLASS_ID:
            return [250, 251, 252]
        case HUNTER_CLASS_ID:
            return [253, 254, 255]
        case PRIEST_CLASS_ID:
            return [256, 257, 258]
        case ROGUE_CLASS_ID:
            return [259, 260, 261]
        case SHAMAN_CLASS_ID:
            return [262, 263, 264]
        case WARLOCK_CLASS_ID:
            return [265, 266, 267]
        case MONK_CLASS_ID:
            return [268, 269, 270]
        case DH_CLASS_ID:
            return [577, 581]
        default:
            return []
    }
}

export function specToName(specId: number): string {
    switch (specId) {
        // MAGE
        case 62:
            return 'Arcane'
        case 63:
            return 'Fire'
        case 64:
            return 'Frost'
        case 1449:
            return 'Initial'
        // PALLY
        case 65:
            return 'Holy'
        case 66:
            return 'Protection'
        case 70:
            return 'Retribution'
        case 1451:
            return 'Initial'
        // WARRIOR
        case 71:
            return 'Arms'
        case 72:
            return 'Fury'
        case 73:
            return 'Protection'
        case 1446:
            return 'Initial'
        // DRUID
        case 102:
            return 'Balance'
        case 103:
            return 'Feral'
        case 104:
            return 'Guardian'
        case 105:
            return 'Restoration'
        case 1447:
            return 'Initial'
        // DEATHKNIGHT
        case 250:
            return 'Blood'
        case 251:
            return 'Frost'
        case 252:
            return 'Unholy'
        case 1455:
            return 'Initial'
        // HUNTER
        case 253:
            return 'Beast Mastery'
        case 254:
            return 'Marksmanship'
        case 255:
            return 'Survival'
        case 1448:
            return 'Initial'
        // PRIEST
        case 256:
            return 'Discipline'
        case 257:
            return 'Holy'
        case 258:
            return 'Shadow'
        case 1452:
            return 'Initial'
        // ROGUE
        case 259:
            return 'Assassination'
        case 260:
            return 'Outlaw'
        case 261:
            return 'Subtlety'
        case 1453:
            return 'Initial'
        // SHAMAN
        case 262:
            return 'Elemental'
        case 263:
            return 'Enhancement'
        case 264:
            return 'Restoration'
        case 1444:
            return 'Initial'
        // WARLOCK
        case 265:
            return 'Affliction'
        case 266:
            return 'Demonology'
        case 267:
            return 'Destruction'
        case 1454:
            return 'Initial'
        // MONK
        case 268:
            return 'Brewmaster'
        case 269:
            return 'Windwalker'
        case 270:
            return 'Mistweaver'
        case 1450:
            return 'Initial'
        // DH
        case 577:
            return 'Havoc'
        case 581:
            return 'Vengeance'
        case 1456:
            return 'Initial'
        default:
            return 'Unknown'
    }
}

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
export enum WowRole {
    Tank,
    Healer,
    Dps,
    Unknown
}

export function specIdToRole(specId: number): WowRole {
    switch (specId) {
        case 65:
        case 256:
        case 257:
        case 270:
        case 105:
        case 264:
            return WowRole.Healer
        case 104:
        case 66:
        case 73:
        case 581:
        case 250:
        case 268:
            return WowRole.Tank
        default:
            return WowRole.Dps
    }
}

const WOW_TANK_SPEC_IDS = ALL_WOW_CLASSES.map((ele: number) => classToSpecIds(ele)).flat().filter((ele: number) => specIdToRole(ele) === WowRole.Tank)
const WOW_HEALER_SPEC_IDS = ALL_WOW_CLASSES.map((ele: number) => classToSpecIds(ele)).flat().filter((ele: number) => specIdToRole(ele) === WowRole.Healer)
const WOW_DPS_SPEC_IDS = ALL_WOW_CLASSES.map((ele: number) => classToSpecIds(ele)).flat().filter((ele: number) => specIdToRole(ele) === WowRole.Dps)

export function roleToSpecIds(role: WowRole): number[] {
    switch (role) {
        case WowRole.Tank:
            return WOW_TANK_SPEC_IDS
        case WowRole.Healer:
            return WOW_HEALER_SPEC_IDS
        case WowRole.Dps:
            return WOW_DPS_SPEC_IDS
        case WowRole.Unknown:
            return []
    }
}