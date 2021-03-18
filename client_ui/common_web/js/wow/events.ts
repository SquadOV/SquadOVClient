export interface WowUserTarget {
    guid: string
    name: string
    flags: number
}

export interface WowDeath  extends WowUserTarget{
    tm: Date
}

export function cleanWowDeathFromJson(e: WowDeath): WowDeath {
    e.tm = new Date(e.tm)
    return e
}

export interface WowResurrection extends WowUserTarget {
    tm: Date
}

export function cleanWowResurrectionFromJson(e: WowResurrection): WowResurrection {
    e.tm = new Date(e.tm)
    return e
}

export interface WowAuraType {
    type: string
}

export interface WowAura {
    targetGuid: string
    targetName: string
    spellId: number
    auraType: WowAuraType
    appliedTm: Date
    removedTm: Date
}

export function cleanWowAuraFromJson(e: WowAura): WowAura {
    e.appliedTm = new Date(e.appliedTm)
    e.removedTm = new Date(e.removedTm)
    return e
}

export interface WowEncounter {
    encounterName: string
    startTm: Date
    endTm: Date
}

export function cleanWowEncounterFromJson(e: WowEncounter): WowEncounter {
    e.startTm = new Date(e.startTm)
    e.endTm = new Date(e.endTm)
    return e
}

export interface WowSpellCast {
    sourceGuid: string
    sourceName: string
    sourceFlags: number
    targetGuid: string | null
    targetName: string | null
    targetFlags: number | null
    castStart: Date | null
    castFinish: Date
    spellId: number
    spellSchool: number
    success: boolean
    instant: boolean
}

export function cleanWowSpellCastFromJson(c: WowSpellCast): WowSpellCast {
    if (!!c.castStart) {
        c.castStart = new Date(c.castStart)
    }
    c.castFinish = new Date(c.castFinish)
    return c
}

export interface WowAuraBreak {
    sourceGuid: string
    sourceName: string
    sourceFlags: number
    targetGuid: string
    targetName: string
    targetFlags: number
    auraId: number
    auraType: WowAuraType
    spellId: number | null
    tm: Date
}

export function cleanWowAuraBreakFromJson(b: WowAuraBreak): WowAuraBreak {
    b.tm = new Date(b.tm)
    return b
}

export interface SerializedWowMatchEvents {
    deaths: WowDeath[]
    auras: WowAura[]
    encounters: WowEncounter[]
    resurrections: WowResurrection[]
    auraBreaks: WowAuraBreak[]
    spellCasts: WowSpellCast[]
}

export function cleanWowMatchEventsFromJson(e: SerializedWowMatchEvents) : SerializedWowMatchEvents {
    e.deaths.forEach(cleanWowDeathFromJson)
    e.auras.forEach(cleanWowAuraFromJson)
    e.encounters.forEach(cleanWowEncounterFromJson)
    e.resurrections.forEach(cleanWowResurrectionFromJson)
    e.auraBreaks.forEach(cleanWowAuraBreakFromJson)
    e.spellCasts.forEach(cleanWowSpellCastFromJson)
    return e
}

export interface UnifiedWowEventContainer {
    tm: Date,
    death?: WowDeath
    resurrect?: WowResurrection
}

export function isWowAuraBuff(a : WowAuraType): boolean {
    return (a.type.toLowerCase() === 'buff')
}