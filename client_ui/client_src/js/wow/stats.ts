export interface WowStatQueryParam {
    psStepSeconds: number,
    psIntervalSeconds: number,
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