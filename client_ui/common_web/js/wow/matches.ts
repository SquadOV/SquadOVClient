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

export interface WowArena extends WowCommonMatch {
    type: string
    winningTeamId: number | null
    matchDurationSeconds: number | null
    newRatings: number[] | null
}

export function getOppositeWowArenaTeam(t: number) : number {
    return (t + 1) % 2
}

export function cleanWowArenaFromJson(c: WowArena): WowArena {
    c.tm = new Date(c.tm)
    if (!!c.finishTime) {
        c.finishTime = new Date(c.finishTime)
    }
    return c
}

export interface GenericWowMatchContainer {
    encounter: WowEncounter | null
    challenge: WowChallenge | null
    arena: WowArena | null
}

export function cleanGenericWowMatchContainerFromJson(c: GenericWowMatchContainer): GenericWowMatchContainer {
    if (!!c.encounter) {
        cleanWowEncounterFromJson(c.encounter)
    }
    
    if (!!c.challenge) {
        cleanWowChallengeFromJson(c.challenge)
    }

    if (!!c.arena) {
        cleanWowArenaFromJson(c.arena)
    }

    return c
}