export interface VodAssociation {
    matchUuid: string
    userUuid: string
    videoUuid: string
    startTime: Date
    endTime: Date
}

export function cleanVodAssocationData(v : VodAssociation) : VodAssociation {
    v.startTime = new Date(v.startTime)
    v.endTime = new Date(v.endTime)
    return v
}