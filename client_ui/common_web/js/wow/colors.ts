import { Color } from '@client/js/color'

export function getFailureColor() : Color {
    return {
        r: 255,
        g: 82,
        b: 82,
    }
}

export function getSuccessColor() : Color {
    return {
        r: 76,
        g: 175,
        b: 80,
    }
}

export function getSelfColor() : Color {
    return {
        r: 255,
        g: 215,
        b: 0,
    }
}

export function getDpsColor(): Color {
    return getSelfColor()
}

export function getHpsColor(): Color {
    return getSuccessColor()
}

export function getDrpsColor(): Color {
    return getFailureColor()
}

import {
    specIdToClassId,
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
    DH_CLASS_ID
} from '@client/js/wow/character'

export function classIdToColor(cls: number): string {
    switch (cls) {
        case WARRIOR_CLASS_ID:
            return 'color-wow-class-warrior'
        case PALLY_CLASS_ID:
            return 'color-wow-class-paladin'
        case HUNTER_CLASS_ID:
            return 'color-wow-class-hunter'
        case ROGUE_CLASS_ID:
            return 'color-wow-class-rogue'
        case PRIEST_CLASS_ID:
            return 'color-wow-class-priest'
        case DK_CLASS_ID:
            return 'color-wow-class-dk'
        case SHAMAN_CLASS_ID:
            return 'color-wow-class-shaman'
        case MAGE_CLASS_ID:
            return 'color-wow-class-mage'
        case WARLOCK_CLASS_ID:
            return 'color-wow-class-warlock'
        case MONK_CLASS_ID:
            return 'color-wow-class-monk'
        case DRUID_CLASS_ID:
            return 'color-wow-class-druid'
        case DH_CLASS_ID:
            return 'color-wow-class-dh'
        default:
            return 'color-self'
    }
}

export function specIdToColor(specId: number): string {
    const classId = specIdToClassId(specId)
    return classIdToColor(classId)
}

const WOW_SCHOOL_NONE = 0x0
const WOW_SCHOOL_PHYSICAL = 0x01
const WOW_SCHOOL_HOLY = 0x02
const WOW_SCHOOL_FIRE = 0x04
const WOW_SCHOOL_NATURE = 0x08
const WOW_SCHOOL_FROST = 0x10
const WOW_SCHOOL_SHADOW = 0x20
const WOW_SCHOOL_ARCANE = 0x40

export function spellSchoolToColor(school: number): string {
    switch (school) {
        case WOW_SCHOOL_PHYSICAL:
            return 'color-wow-school-physical'
        case WOW_SCHOOL_HOLY:
            return 'color-wow-school-holy'
        case WOW_SCHOOL_FIRE:
            return 'color-wow-school-fire'
        case WOW_SCHOOL_NATURE:
            return 'color-wow-school-nature'
        case WOW_SCHOOL_FROST:
            return 'color-wow-school-frost'
        case WOW_SCHOOL_SHADOW:
            return 'color-wow-school-shadow'
        case WOW_SCHOOL_ARCANE:
            return 'color-wow-school-arcane'
        case WOW_SCHOOL_NONE:
        default:
            return 'color-wow-school-none'
    }
}

const WOW_QUALITY_POOR = 0
const WOW_QUALITY_COMMON = 1
const WOW_QUALITY_UNCOMMON = 2
const WOW_QUALITY_RARE = 3
const WOW_QUALITY_EPIC = 4
const WOW_QUALITY_LEGENDARY = 5
const WOW_QUALITY_ARTIFCAT = 6
const WOW_QUALITY_HEIRLOOM = 7
const WOW_QUALITY_TOKEN = 8

export function wowItemQualityToColor(quality: number): string {
    switch (quality) {
        case WOW_QUALITY_POOR:
            return 'color-wow-quality-poor'
        case WOW_QUALITY_UNCOMMON:
            return 'color-wow-quality-uncommon'
        case WOW_QUALITY_RARE:
            return 'color-wow-quality-rare'
        case WOW_QUALITY_EPIC:
            return 'color-wow-quality-epic'
        case WOW_QUALITY_LEGENDARY:
            return 'color-wow-quality-legendary'
        case WOW_QUALITY_ARTIFCAT:
            return 'color-wow-quality-artifact'
        case WOW_QUALITY_HEIRLOOM:
            return 'color-wow-quality-heirloom'
        case WOW_QUALITY_TOKEN:
            return 'color-wow-quality-token'
        case WOW_QUALITY_COMMON:
        default:
            return 'color-wow-quality-common'
    }
}