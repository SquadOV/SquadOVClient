import data from '@client/js/aimlab/data.json'

interface AimlabRawData {
    tasks: {
        [taskName : string] : {
            name : string
            icon : string
            defaultMode: number
            mode: {
                [mode: number] : {
                    name : string
                    scoreStat: string
                    extraStats: string[] | undefined
                }
            }
        }
    },
}

export class AimlabContent {
    data : AimlabRawData
    validTaskKeys: string[]

    constructor(inputData : any) {
        this.data = inputData

        this.validTaskKeys = []
        for (const key of Object.keys(this.data.tasks)) {
            this.validTaskKeys.push(key)
        }
    }

    isTaskSupported(id : string) : boolean {
        return (id in this.data.tasks)
    }

    isTaskModeSupported(id: string, mode : number) : boolean {
        if (!this.isTaskSupported(id)) {
            return false
        }
        let task = this.data.tasks[id]!
        return (mode in task.mode)
    }

    getTaskName(id : string) : string {
        if (!this.isTaskSupported(id)) {
            return 'Unknown'
        }
        return this.data.tasks[id].name
    }

    getTaskMode(id : string, mode : number) : string {
        if (!this.isTaskSupported(id)) {
            return 'Unknown'
        }

        let task = this.data.tasks[id]!
        if (!(mode in task.mode)) {
            return 'Standard'
        }
        
        return task.mode[mode].name
    }

    getTaskIcon(id : string) : string {
        if (!this.isTaskSupported(id)) {
            return 'assets/aimlab-logo.png'
        }
        return this.data.tasks[id].icon
    }

    getTaskCommonScoreStat(id : string, mode : number) : string | undefined {
        if (!this.isTaskSupported(id)) {
            return undefined
        }
        return this.data.tasks[id].mode[mode].scoreStat
    }

    getTaskExtraStats(id : string, mode : number) : string[] | undefined {
        if (!this.isTaskModeSupported(id, mode)) {
            return undefined
        }
        return this.data.tasks[id].mode[mode].extraStats
    }
}

let content : AimlabContent | null = null

export function getAimlabContent() : AimlabContent {
    if (!content) {
        content = new AimlabContent(data)
    }

    return content!
}