export interface WowStatQueryParam {
    psStepSeconds: number,
    // UNIX time in milliseconds 
    start: number,
    end: number
}

export interface WowStatDatum {
    tm: number
    value: number
}

export interface WowMatchStatContainer {
    [guid: string]: WowStatDatum[] | undefined
}

export interface WowStatItem {
    guid: string
    value: number
}

export interface WowMatchStatSummaryData {
    damageDealt: WowStatItem[]
    damageReceived: WowStatItem[]
    heals: WowStatItem[]
}