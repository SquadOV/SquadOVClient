export interface WowCommonMatch {
    matchUuid: string
    tm: Date
    combatantsKey: string
    instanceId: number
    finishTime: Date | null | undefined
    success: boolean
    build: string
}

export interface WowEncounter extends WowCommonMatch {
    encounterId: number
    encounterName: string
    difficulty: number
    numPlayers: number
}

export function cleanWowEncounterFromJson(e: WowEncounter): WowEncounter {
    e.tm = new Date(e.tm)
    if (!!e.finishTime) {
        e.finishTime = new Date(e.finishTime)
    }
    return e
}

export interface WowChallenge extends WowCommonMatch {
    challengeName: string
    keystoneLevel: number
    timeMs: number
}

export function cleanWowChallengeFromJson(c: WowChallenge): WowChallenge {
    c.tm = new Date(c.tm)
    if (!!c.finishTime) {
        c.finishTime = new Date(c.finishTime)
    }
    return c
}