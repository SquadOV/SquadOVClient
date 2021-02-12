import parse from 'date-fns/parse'

export interface AllStats {
    aimlab?: AimlabStats
}

interface AimlabModeStatData<T> {
    ultimate?: T[]
    speed?: T[]
    precision: T[]
}

export interface AimlabStats {
    gridshot?: AimlabModeStatData<AimlabSparData>
    spidershot?: AimlabModeStatData<AimlabSparData>
    microshot?: AimlabModeStatData<AimlabSparData>
    sixshot?: AimlabModeStatData<AimlabSparData>
    microflex?: AimlabModeStatData<AimlabSparData>
    motionshot?: AimlabModeStatData<AimlabSparData>
    multishot?: AimlabModeStatData<AimlabSparData>

    detection?: AimlabModeStatData<AimlabDetectionData>
    
    decisionshot?: AimlabModeStatData<AimlabDecisionshotData>
    
    strafetrack?: AimlabModeStatData<AimlabTrackData>
    circletrack?: AimlabModeStatData<AimlabTrackData>

    strafeshot?: AimlabModeStatData<AimlabErbData>
    circleshot?: AimlabModeStatData<AimlabErbData>
    
    linetrace?: AimlabModeStatData<AimlabLinetraceData>
    multilinetrace?: AimlabModeStatData<AimlabLinetraceData>

    pentakill?: AimlabModeStatData<AimlabPentakillData>
}

export interface AimlabCommonStatData {
    date?: Date | null
    datetime?: Date | null
    time?: Date | null
    id?: number | null
    version?: string | null
    score?: number
}

export interface AimlabSparData extends AimlabCommonStatData {
    kill?: number
    ttk?: number
    acc?: number
}

export interface AimlabDetectionData extends AimlabCommonStatData {
}

export interface AimlabDecisionshotData extends AimlabCommonStatData {
}

export interface AimlabTrackData extends AimlabCommonStatData {
}

export interface AimlabErbData extends AimlabCommonStatData {
}

export interface AimlabLinetraceData extends AimlabCommonStatData {
}

export interface AimlabPentakillData extends AimlabCommonStatData {
}

function cleanAimlabCommonStatDataFromJson(st: AimlabCommonStatData) : AimlabCommonStatData {
    if (!!st.date) {
        st.date = new Date(st.date)
    }

    if (!!st.datetime) {
        st.datetime = new Date(st.datetime)
    }

    if (!!st.time) {
        //@ts-ignore
        st.time = parse(st.time, 'HH:mm:ss', new Date())
    }
    return st
}

function cleanAimlabModeStatDataFromJson<T>(st: AimlabModeStatData<T>) : AimlabModeStatData<T> {
    if (!!st.ultimate) {
        st.ultimate.forEach(cleanAimlabCommonStatDataFromJson)
    }

    if (!!st.precision) {
        st.precision.forEach(cleanAimlabCommonStatDataFromJson)
    }

    if (!!st.speed) {
        st.speed.forEach(cleanAimlabCommonStatDataFromJson)
    }
    return st
}

function cleanAimlabStatsFromJson(st: AimlabStats) : AimlabStats {
    if (!!st.gridshot) {
        cleanAimlabModeStatDataFromJson(st.gridshot)
    }

    if (!!st.spidershot) {
        cleanAimlabModeStatDataFromJson(st.spidershot)
    }

    if (!!st.microshot) {
        cleanAimlabModeStatDataFromJson(st.microshot)
    }

    if (!!st.sixshot) {
        cleanAimlabModeStatDataFromJson(st.sixshot)
    }

    if (!!st.microflex) {
        cleanAimlabModeStatDataFromJson(st.microflex)
    }

    if (!!st.motionshot) {
        cleanAimlabModeStatDataFromJson(st.motionshot)
    }

    if (!!st.multishot) {
        cleanAimlabModeStatDataFromJson(st.multishot)
    }

    if (!!st.detection) {
        cleanAimlabModeStatDataFromJson(st.detection)
    }

    if (!!st.decisionshot) {
        cleanAimlabModeStatDataFromJson(st.decisionshot)
    }

    if (!!st.strafetrack) {
        cleanAimlabModeStatDataFromJson(st.strafetrack)
    }

    if (!!st.circletrack) {
        cleanAimlabModeStatDataFromJson(st.circletrack)
    }

    if (!!st.strafeshot) {
        cleanAimlabModeStatDataFromJson(st.strafeshot)
    }

    if (!!st.circleshot) {
        cleanAimlabModeStatDataFromJson(st.circleshot)
    }

    if (!!st.linetrace) {
        cleanAimlabModeStatDataFromJson(st.linetrace)
    }

    if (!!st.multilinetrace) {
        cleanAimlabModeStatDataFromJson(st.multilinetrace)
    }

    if (!!st.pentakill) {
        cleanAimlabModeStatDataFromJson(st.pentakill)
    }
    return st
}

export function cleanAllStatsFromJson(st: AllStats) : AllStats {
    if (!!st.aimlab) {
        cleanAimlabStatsFromJson(st.aimlab)
    }
    return st
}