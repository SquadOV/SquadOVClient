export interface AimlabTaskData {
    id : number
    userId: number
    klutchId: string
    matchUuid: string
    taskName : string
    mode : number
    score : number
    createDate : Date
    version : string
    rawData : string
}

export function cleanAimlabTaskData(d : AimlabTaskData) : AimlabTaskData {
    d.createDate = new Date(d.createDate)
    return d
}