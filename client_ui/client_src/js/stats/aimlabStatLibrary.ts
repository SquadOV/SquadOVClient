import {
    StatFolder,
    StatOption,
    StatObject,
    StatOptionInstance,
    StatInstance,
    createStatGroupFunctionOption,
    createStatSortDirectionOption
} from '@client/js/stats/statPrimitives'
import { GraphqlStatsQueryBuilder } from '@client/js/graphql/graphql'
import {
    AllStats,
    AimlabSparData,
    AimlabCommonStatData
} from '@client/js/graphql/schema'
import { StatXYSeriesData } from '@client/js/stats/seriesData'

export enum AimlabGrouping {
    AgId = "AGID",
    AgDate = "AGDATE",
    AgDateTime = "AGDATETIME",
    AgTime = "AGTIME",
    AgVersion = "AGVERSION",
}

export function createAimlabGroupOption(): StatOption {
    return {
        name: 'Group',
        id: 'group',
        choices: [
            {
                name: 'ID',
                value: AimlabGrouping.AgId,
            },
            {
                name: 'Date',
                value: AimlabGrouping.AgDate,
            },
            {
                name: 'Date Time',
                value: AimlabGrouping.AgDateTime,
            },
            {
                name: 'Time',
                value: AimlabGrouping.AgTime,
            },
            {
                name: 'Version',
                value: AimlabGrouping.AgVersion,
            },
        ],
        createDefault: () => {
            return AimlabGrouping.AgId
        }
    }
}

function getCommonAimLabOptions() : StatOption[] {
    return [
        createAimlabGroupOption(),
        createStatGroupFunctionOption(),
        createStatSortDirectionOption()
    ]
}

function createGenericAimlabAddToGraphqlBuilder(task : string, mode: string, stat: string) : <T>(b: GraphqlStatsQueryBuilder<T>, opts: StatOptionInstance) => void {
    return <T>(b: GraphqlStatsQueryBuilder<T>, opts: StatOptionInstance) => {
        //@ts-ignore
        b.aimlab()[task]()[mode]({
            func: opts.data['func'],
            group: opts.data['group'],
            sort: opts.data['sort'],
        })[stat]()
    }
}

function createGenericAimlabParseResponseIntoInstance<T extends AimlabCommonStatData>(task: string, mode: string, stat: string): (stats: AllStats, inst: StatInstance) => void {
    return (stats: AllStats, inst: StatInstance) => {
        let aimlab = stats.aimlab
        if (!aimlab) {
            return
        }

        //@ts-ignore
        let taskData = aimlab[task]
        if (!taskData) {
            return
        }

        //@ts-ignore
        let modeData: T[] = taskData[mode]
        if (!modeData) {
            return
        }

        if (modeData.length == 0) {
            return
        }

        // Use a single sample to determine which element we want to use for the
        // X-series data.
        let sample = modeData[0]
        let xKey = ''
        let xType = ''
        if (!!sample.date) {
            xKey = 'date'
            xType = 'time'
        } else if (!!sample.datetime) {
            xKey = 'datetime'
            xType = 'time'
        } else if (!!sample.id) {
            xKey = 'id'
            xType = 'category'
        } else if (!!sample.time) {
            xKey = 'time'
            xType = 'time'
        } else if (!!sample.version) {
            xKey = 'version'
            xType = 'category'
        } else {
            return
        }

        //@ts-ignore
        let xData: any[] = modeData.map((ele: T) => ele[xKey])
        //@ts-ignore
        let yData : any[] = modeData.map((ele: T) => ele[stat])
        inst.data = new StatXYSeriesData(xData, yData, xType, inst.name)
    }
}

function createAimlabSparStats(task: string, mode: string) : { [id: string] : StatObject } {
    return {
        'score': {
            name: 'Score',
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'score'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'score')
        },
        'kill': {
            name: 'Kills',
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'kill'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'kill')
        },
        'ttk': {
            name: 'Time-to-Kill (ms)',
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'ttk'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'ttk')
        },
        'acc': {
            name: 'Accuracy (%)',
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'acc'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'acc')
        }
    }
}

export function createAimlabStatFolder() : StatFolder {
    return {
        name: 'Aimlab',
        childFolders: {
            'gridshot': {
                name: 'Gridshot',
                childFolders: {
                    'ultimate': {
                        name: 'Ultimate',
                        stats: createAimlabSparStats('gridshot', 'ultimate')
                    },
                    'speed': {
                        name: 'Speed',
                        stats: createAimlabSparStats('gridshot', 'speed')
                    },
                    'precision': {
                        name: 'Precision',
                        stats: createAimlabSparStats('gridshot', 'precision')
                    }
                }
            }
        }
    }
}