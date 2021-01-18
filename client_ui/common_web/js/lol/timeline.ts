export interface LolMatchPosition {
    x: number
    y: number
}

export interface LolMatchParticipantFrame {
    participantId: number
    minionsKilled: number
    totalGold: number
    level: number
    xp: number
    currentGold: number
    jungleMinionsKilled: number
    position: LolMatchPosition | null
}

export interface LolMatchEvent {
    laneType: string | null
    skillSlot: string | null
    ascendedType: string | null
    creatorId: number | null
    afterId: number | null
    eventType: string | null
    type: string
    levelUpType: string | null
    wardType: string | null
    participantId: number | null
    towerType: string | null
    itemId: number | null
    beforeId: number | null
    monsterType: string | null
    monsterSubType: string | null
    teamId: number | null
    position: LolMatchPosition | null
    killerId: number | null
    timestamp: number
    assistingParticipantIds: number[] | null
    buildingType: string | null
    victimId: number | null
}

export interface LolMatchFrame {
    participantFrames: { [id: string] : LolMatchParticipantFrame | undefined }
    events: LolMatchEvent[]
    timestamp: number
}

export interface LolMatchTimeline {
    frames: LolMatchFrame[]
    frameInterval: number
}

export enum LolStaticTargets {
    EDragon,
    EDragonAir,
    EDragonFire,
    EDragonEarth,
    EDragonWater,
    EDragonElder,
    EBaron,
    ERiftHerald,
    ETower,
    EInhibitor,
    ESightWard,
    EControlWard,
    EYellowTrinket,
    EBlueTrinket,
    EWard
}