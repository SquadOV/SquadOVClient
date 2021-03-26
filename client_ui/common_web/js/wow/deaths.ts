export interface WowDeathRecapEvent {
    tm: Date
    diffMs: number
    diffHp: number
    spellId: number | null
}

export interface WowDeathRecap {
    hpEvents: WowDeathRecapEvent[]
}

export function cleaWowDeathRecapEventFromJson(e: WowDeathRecapEvent): WowDeathRecapEvent {
    e.tm = new Date(e.tm)
    return e
}

export function cleanWowDeathRecapFromJson(d: WowDeathRecap): WowDeathRecap {
    d.hpEvents.forEach(cleaWowDeathRecapEventFromJson)
    return d
}