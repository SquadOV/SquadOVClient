export interface AimlabTaskData {
    id : number
    taskName : string
    mode : number
    score : number
    createDate : Date
    version : string
    rawData : string
    vodPath : string | null
}

export function cleanAimlabTaskData(d : AimlabTaskData) : AimlabTaskData {
    d.createDate = new Date(d.createDate)
    return d
}