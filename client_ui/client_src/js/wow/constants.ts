// Affiliation
const COMBATLOG_OBJECT_AFFILIATION_MINE         = 0x00000001;
const COMBATLOG_OBJECT_AFFILIATION_PARTY        = 0x00000002;
const COMBATLOG_OBJECT_AFFILIATION_RAID         = 0x00000004;
const COMBATLOG_OBJECT_AFFILIATION_OUTSIDER     = 0x00000008;
const COMBATLOG_OBJECT_AFFILIATION_MASK         = 0x0000000F;
// Reaction
const COMBATLOG_OBJECT_REACTION_FRIENDLY        = 0x00000010;
const COMBATLOG_OBJECT_REACTION_NEUTRAL         = 0x00000020;
const COMBATLOG_OBJECT_REACTION_HOSTILE         = 0x00000040;
const COMBATLOG_OBJECT_REACTION_MASK            = 0x000000F0;
// Ownership
const COMBATLOG_OBJECT_CONTROL_PLAYER           = 0x00000100;
const COMBATLOG_OBJECT_CONTROL_NPC              = 0x00000200;
const COMBATLOG_OBJECT_CONTROL_MASK             = 0x00000300;
// Unit type
const COMBATLOG_OBJECT_TYPE_PLAYER              = 0x00000400;
const COMBATLOG_OBJECT_TYPE_NPC                 = 0x00000800;
const COMBATLOG_OBJECT_TYPE_PET                 = 0x00001000;
const COMBATLOG_OBJECT_TYPE_GUARDIAN            = 0x00002000;
const COMBATLOG_OBJECT_TYPE_OBJECT              = 0x00004000;
const COMBATLOG_OBJECT_TYPE_MASK                = 0x0000FC00;
// Special cases (non-exclusive)
const COMBATLOG_OBJECT_TARGET                   = 0x00010000;
const COMBATLOG_OBJECT_FOCUS                    = 0x00020000;
const COMBATLOG_OBJECT_MAINTANK                 = 0x00040000;
const COMBATLOG_OBJECT_MAINASSIST               = 0x00080000;
const COMBATLOG_OBJECT_NONE                     = 0x80000000;
const COMBATLOG_OBJECT_SPECIAL_MASK             = 0xFFFF0000;

export function isObjectFriendly(flags: number): boolean {
    return ((flags & COMBATLOG_OBJECT_REACTION_FRIENDLY) > 0)
}

export function isObjectHostile(flags: number): boolean {
    return ((flags & COMBATLOG_OBJECT_REACTION_HOSTILE) > 0)
}

export function isObjectNeutral(flags: number): boolean {
    return ((flags & COMBATLOG_OBJECT_REACTION_NEUTRAL) > 0)
}

export function isObjectPlayer(flags: number): boolean {
    return ((flags & COMBATLOG_OBJECT_TYPE_PLAYER) > 0)
        && ((flags & COMBATLOG_OBJECT_CONTROL_PLAYER) > 0)
}

export function isObjectNpc(flags: number): boolean {
    return ((flags & COMBATLOG_OBJECT_CONTROL_NPC) > 0)
}

export function isObjectPet(flags: number): boolean {
    return ((flags & COMBATLOG_OBJECT_TYPE_PET) > 0)
        || ((flags & COMBATLOG_OBJECT_TYPE_GUARDIAN) > 0)
}

export const WOW_OVERRIDE_BOSS_NAMES: string[] = [
    "Margrave Stradama"   
]