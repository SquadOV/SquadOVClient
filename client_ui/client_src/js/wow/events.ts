export interface WowDeath {
    guid: string
    name: string
    flags: number
    tm: Date
}

export function cleanWowDeathFromJson(e: WowDeath): WowDeath {
    e.tm = new Date(e.tm)
    return e
}

export interface WowAura {
    targetGuid: string
    targetName: string
    spellId: number
    auraType: {
        type: string
    }
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

export interface SerializedWowMatchEvents {
    deaths: WowDeath[]
    auras: WowAura[]
    encounters: WowEncounter[]
}

export function cleanWowMatchEventsFromJson(e: SerializedWowMatchEvents) : SerializedWowMatchEvents {
    e.deaths.forEach(cleanWowDeathFromJson)
    e.auras.forEach(cleanWowAuraFromJson)
    e.encounters.forEach(cleanWowEncounterFromJson)
    return e
}

export interface UnifiedWowEventContainer {
    tm: Date,
    death?: WowDeath
}
