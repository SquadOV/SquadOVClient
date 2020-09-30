export interface GraphqlQuerySchema {
    stats?: AllStats
}

export interface AllStats {
    aimlab?: {
        gridshot?: AimlabSparData[]
        spidershot: AimlabSparData[]
        microshot: AimlabSparData[]
        sixshot: AimlabSparData[]
        microflex: AimlabSparData[]
        motionshot: AimlabSparData[]
        multishot: AimlabSparData[]
    
        detection: AimlabDetectionData[]
        
        decisionshot: AimlabDecisionshotData[]
        
        strafetrack: AimlabTrackData[]
        circletrack: AimlabTrackData[]
    
        strafeshot: AimlabErbData[]
        circleshot: AimlabErbData[]
        
        linetrace: AimlabLinetraceData[]
        multilinetrace: AimlabLinetraceData[]
    
        pentakill: AimlabPentakillData[]
    }
}

export interface AimlabSparData {
    x: string
    score: number
    kill: number
    ttk: number
    acc: number
}

export interface AimlabDetectionData {
    x: string
    score: number 
}

export interface AimlabDecisionshotData {
    x: string
    score: number 
}

export interface AimlabTrackData {
    x: string
    score: number 
}

export interface AimlabErbData {
    x: string
    score: number 
}

export interface AimlabLinetraceData {
    x: string
    score: number 
}

export interface AimlabPentakillData {
    x: string
    score: number  
}