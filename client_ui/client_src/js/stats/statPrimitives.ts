import { GraphqlStatsQueryBuilder } from '@client/js/graphql/graphql'
import { AllStats } from '@client/js/graphql/schema'
import { StatXYSeriesData } from '@client/js/stats/seriesData'

export interface StatOptionChoice {
    name: string
    value: any
}

export interface StatOption {
    name: string
    id: string
    choices?: StatOptionChoice[]
    createDefault: () => any
}

export interface StatNode {
    name: string
    parent? : StatNode
}

export interface StatObject extends StatNode {
    availableOptions: StatOption[]
    addToGraphqlBuilder<T>(b: GraphqlStatsQueryBuilder<T>, opts: StatOptionInstance): void
    parseResponseIntoInstance(stats: AllStats, inst: StatInstance): void
}

export interface StatFolder extends StatNode {
    childFolders?: { [id: string] : StatFolder }
    stats? : { [id: string] : StatObject }
}

export interface StatOptionInstance {
    options: StatOption[]
    data: {[nm: string]: any}
}

export interface StatInstance {
    id: string
    name: string
    obj: StatObject
    options: StatOptionInstance
    data?: StatXYSeriesData
}

export enum StatGroupFunctions {
    Avg = "AVG",
    Min = "MIN",
    Max = "MAX"
}

export function createStatGroupFunctionOption(): StatOption {
    return {
        name: 'Function',
        id: 'func',
        choices: [
            {
                name: 'Average',
                value: StatGroupFunctions.Avg,
            },
            {
                name: 'Min',
                value: StatGroupFunctions.Min,
            },
            {
                name: 'Max',
                value: StatGroupFunctions.Max,
            }
        ],
        createDefault: () => {
            return StatGroupFunctions.Avg
        }
    }
}

export enum StatSortDirection {
    Asc = "ASC",
    Desc = "DESC"
}

export function createStatSortDirectionOption(): StatOption {
    return {
        name: 'Sort',
        id: 'sort',
        choices: [
            {
                name: 'Ascending',
                value: StatSortDirection.Asc
            },
            {
                name: 'Descending',
                value: StatSortDirection.Desc
            }
        ],
        createDefault: () => {
            return StatSortDirection.Asc
        }
    }
}