export interface WowCommonMatch {
    matchUuid: string
    tm: Date
    combatantsKey: string
    instanceId: number
    finishTime: Date | null | undefined
    success: boolean
    build: string
}

export interface WowBossStatus {
    name: string
    npcId: number
    currentHp: number | null
    maxHp: number | null
}

export interface WowEncounter extends WowCommonMatch {
    encounterId: number
    encounterName: string
    difficulty: number
    numPlayers: number
    boss: WowBossStatus[]
    pullNumber: number | null
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

export enum WowInstanceType {
    NotInstanced,
    PartyDungeon,
    RaidDungeon,
    PVPBattlefield,
    ArenaBattlefield,
    Scenario,
    Unknown
}

export const ALL_WOW_INSTANCE_TYPES = [
    WowInstanceType.PartyDungeon,
    WowInstanceType.RaidDungeon,
    WowInstanceType.PVPBattlefield,
    WowInstanceType.ArenaBattlefield,
    WowInstanceType.Scenario,
]

export function wowInstanceTypeToName(t: WowInstanceType): string {
    switch (t) {
        case WowInstanceType.PartyDungeon:
            return 'Dungeon'
        case WowInstanceType.RaidDungeon:
            return 'Raid'
        case WowInstanceType.PVPBattlefield:
            return 'Battlegrounds'
        case WowInstanceType.ArenaBattlefield:
            return 'Arena'
        case WowInstanceType.Scenario:
            return 'Scenarios'
        default:
            return 'Unknown'
    }
}

export interface WowInstance extends WowCommonMatch {
    instanceType: WowInstanceType
}

export function cleanWowInstanceFromJson(c: WowInstance): WowInstance {
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
    instance: WowInstance | null
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

    if (!!c.instance) {
        cleanWowInstanceFromJson(c.instance)
    }

    return c
}