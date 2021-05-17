import {
    StatFolder,
    StatOption,
    StatObject,
    StatOptionInstance,
    StatInstance,
    createStatGroupFunctionOption,
    createStatSortDirectionOption,
    StatPermission
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
            return AimlabGrouping.AgDate
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
        let xSubtype = ''
        if (!!sample.date) {
            xKey = 'date'
            xType = 'time'
            xSubtype = 'date'
        } else if (!!sample.datetime) {
            xKey = 'datetime'
            xType = 'time'
            xSubtype = 'datetime'
        } else if (!!sample.id) {
            xKey = 'id'
            xType = 'category'
        } else if (!!sample.time) {
            xKey = 'time'
            xType = 'time'
            xSubtype = 'time'
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
        inst.data = new StatXYSeriesData(xData, yData, xType, xSubtype, inst.name)
    }
}

function createGenericAimlabStats(task: string, mode: string, perm: StatPermission) : { [id: string] : StatObject } {
    return {
        'score': {
            name: 'Score',
            permission: perm,
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'score'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabCommonStatData>(task, mode, 'score')
        },
    }
}

function createAimlabSparStats(task: string, mode: string, perm: StatPermission) : { [id: string] : StatObject } {
    return {
        'score': {
            name: 'Score',
            permission: perm,
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'score'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'score')
        },
        'kill': {
            name: 'Kills',
            permission: perm,
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'kill'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'kill')
        },
        'ttk': {
            name: 'Time-to-Kill (ms)',
            permission: perm,
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'ttk'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'ttk')
        },
        'acc': {
            name: 'Accuracy (%)',
            permission: perm,
            availableOptions: [...getCommonAimLabOptions()],
            addToGraphqlBuilder: createGenericAimlabAddToGraphqlBuilder(task, mode, 'acc'),
            parseResponseIntoInstance: createGenericAimlabParseResponseIntoInstance<AimlabSparData>(task, mode, 'acc')
        }
    }
}

enum AimlabModeFlags {
    None = 0,
    Ultimate = 1,
    Precision = 2,
    Speed = 4,
    All = 7
}

function createStandardAimlabStatFolder(id: string, name: string, flags: number, fn: (a: string, b: string, c: StatPermission) => { [id: string] : StatObject }, perm: StatPermission): StatFolder {
    let children : { [id: string] : StatFolder }= {}
    if (flags & AimlabModeFlags.Ultimate) {
        children['ultimate'] = {
            name: 'Ultimate',
            stats: fn(id, 'ultimate', perm)
        }
    }

    if (flags & AimlabModeFlags.Precision) {
        children['precision'] = {
            name: 'Precision',
            stats: fn(id, 'precision', perm)
        }
    }

    if (flags & AimlabModeFlags.Speed) {
        children['speed'] = {
            name: 'Speed',
            stats: fn(id, 'speed', perm)
        }
    }

    return {
        name: name,
        childFolders: children
    }
}

export function createAimlabStatFolder() : StatFolder {
    return {
        name: 'Aimlab',
        childFolders: {
            'gridshot': createStandardAimlabStatFolder('gridshot', 'Gridshot', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabGridshot),
            'spidershot': createStandardAimlabStatFolder('spidershot', 'Spidershot', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabSpidershot),
            'microshot': createStandardAimlabStatFolder('microshot', 'Microshot', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabMicroshot),
            'sixshot': createStandardAimlabStatFolder('sixshot', 'Sixshot', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabSixshot),
            'microflex': createStandardAimlabStatFolder('microflex', 'Microflex', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabMicroflex),
            'motionshot': createStandardAimlabStatFolder('motionshot', 'Motionshot', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabMotionshot),
            'multishot': createStandardAimlabStatFolder('multishot', 'Multishot', AimlabModeFlags.All, createAimlabSparStats, StatPermission.AimlabMultishot),
            'detection': createStandardAimlabStatFolder('detection', 'Detection', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabDetection),
            'decisionshot': createStandardAimlabStatFolder('decisionshot', 'Decisionshot', AimlabModeFlags.All, createGenericAimlabStats, StatPermission.AimlabDecisionshot),
            'strafetrack': createStandardAimlabStatFolder('strafetrack', 'Strafetrack', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabStrafetrack),
            'circletrack': createStandardAimlabStatFolder('circletrack', 'Circletrack', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabCircletrack),
            'strafeshot': createStandardAimlabStatFolder('strafeshot', 'Strafeshot', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabStrafeshot),
            'circleshot': createStandardAimlabStatFolder('circleshot', 'Circleshot', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabCircleshot),
            'linetrace': createStandardAimlabStatFolder('linetrace', 'Linetrace', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabLinetrace),
            'multilinetrace': createStandardAimlabStatFolder('multilinetrace', 'Multilinetrace', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabMultilinetrace),
            'pentakill': createStandardAimlabStatFolder('pentakill', 'Pentakill', AimlabModeFlags.Ultimate, createGenericAimlabStats, StatPermission.AimlabPentakill),
        }
    }
}