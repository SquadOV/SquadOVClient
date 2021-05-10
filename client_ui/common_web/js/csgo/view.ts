export interface CsgoView {
    viewUuid: string
    matchUuid: string | null
    userId: number
    hasGsi: boolean
    hasDemo: boolean
    map: string
    mode: string
    gameServer: string
    startTime: Date
    stopTime: Date | null
}

export function cleanCsgoViewFromJson(v: CsgoView): CsgoView {
    v.startTime = new Date(v.startTime)
    if (!!v.stopTime) {
        v.stopTime = new Date(v.stopTime)
    }
    return v
}