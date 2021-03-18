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

export function classIdToColor(cls: number): Color {
    switch (cls) {
        case WARRIOR_CLASS_ID:
            return {
                r: 198,
                g: 155,
                b: 109
            }
        case PALLY_CLASS_ID:
            return {
                r: 244,
                g: 140,
                b: 186
            }
        case HUNTER_CLASS_ID:
            return {
                r: 170,
                g: 211,
                b: 114
            }
        case ROGUE_CLASS_ID:
            return {
                r: 255,
                g: 244,
                b: 104
            }
        case PRIEST_CLASS_ID:
            return {
                r: 255,
                g: 255,
                b: 255
            }
        case DK_CLASS_ID:
            return {
                r: 196,
                g: 30,
                b: 58
            }
        case SHAMAN_CLASS_ID:
            return {
                r: 0,
                g: 112,
                b: 221
            }
        case MAGE_CLASS_ID:
            return {
                r: 63,
                g: 199,
                b: 235
            }
        case WARLOCK_CLASS_ID:
            return {
                r: 135,
                g: 136,
                b: 238
            }
        case MONK_CLASS_ID:
            return {
                r: 0,
                g: 255,
                b: 152
            }
        case DRUID_CLASS_ID:
            return {
                r: 255,
                g: 124,
                b: 10
            }
        case DH_CLASS_ID:
            return {
                r: 163,
                g: 48,
                b: 201
            }
        default:
            return getSelfColor()
    }
}

export function specIdToColor(specId: number): Color {
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

export function spellSchoolToColor(school: number): Color {
    switch (school) {
        case WOW_SCHOOL_PHYSICAL:
            return {
                r: 255,
                g: 255,
                b: 0
            }
        case WOW_SCHOOL_HOLY:
            return {
                r: 255,
                g: 229,
                b: 127
            }
        case WOW_SCHOOL_FIRE:
            return {
                r: 255,
                g: 127,
                b: 0
            }
        case WOW_SCHOOL_NATURE:
            return {
                r: 76,
                g: 255,
                b: 76
            }
        case WOW_SCHOOL_FROST:
            return {
                r: 127,
                g: 255,
                b: 255,
            }
        case WOW_SCHOOL_SHADOW:
            return {
                r: 127,
                g: 127,
                b: 255,
            }
        case WOW_SCHOOL_ARCANE:
            return {
                r: 255,
                g: 127,
                b: 255,
            }
        case WOW_SCHOOL_NONE:
        default:
            return {
                r: 255,
                g: 255,
                b: 255
            }
    }
}