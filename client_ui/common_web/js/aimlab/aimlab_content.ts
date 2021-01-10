import fs from 'fs'

const path = require('path')
const aimlabContentFile =  process.env.NODE_ENV === 'production' ? path.join(process.resourcesPath, 'assets/aimlab/content.json') : 'assets/aimlab/content.json'

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

    constructor(data : any) {
        this.data = JSON.parse(data)
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
        let data = fs.readFileSync(aimlabContentFile)
        content = new AimlabContent(data)
    }

    return content!
}