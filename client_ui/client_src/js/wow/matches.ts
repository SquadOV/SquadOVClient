export interface WowEncounter {
    matchUuid: string
    tm: Date
    combatantsKey: string
    encounterId: number
    encounterName: string
    difficulty: number
    numPlayers: number
    instanceId: number
    finishTime: Date | null | undefined
    success: boolean
    build: string
}

export function cleanWowEncounterFromJson(e: WowEncounter): WowEncounter {
    e.tm = new Date(e.tm)
    if (!!e.finishTime) {
        e.finishTime = new Date(e.finishTime)
    }
    return e
}

export interface WowChallenge {
    matchUuid: string
    tm: Date
    combatantsKey: string
    challengeName: string
    instanceId: number
    keystoneLevel: number
    finishTime: Date | null | undefined
    timeMs: number
    success: boolean
    build: string
}

export function cleanWowChallengeFromJson(c: WowChallenge): WowChallenge {
    c.tm = new Date(c.tm)
    if (!!c.finishTime) {
        c.finishTime = new Date(c.finishTime)
    }
    return c
}