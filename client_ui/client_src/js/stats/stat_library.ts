import fs from 'fs'
import { GraphqlPathData, GraphqlSingleNodePath, GraphqlAlias, GraphqlVariable } from '@client/js/graphql/graphql'
const path = require('path')
const libFname = process.env.NODE_ENV === 'development' ? 'assets/stats/library.json' : path.join(process.resourcesPath, 'assets/stats/library.json')

export interface StatValue {
    text: string
    value: any
    default?: boolean
    type?: string
}

export interface StatOption {
    id: string
    name: string
    variable: string
    isEnum? : boolean
    values: StatValue[]
}

export interface StatOptionValueMap {
    options: StatOption[]
    values: {[ optId : string ] : StatValue}
}

export function createGraphqlVariables(map : StatOptionValueMap) : GraphqlVariable[] {
    return map.options.map((ele : StatOption) => {
        return {
            key: ele.variable,
            value: map.values[ele.id].value,
        }
    })    
}

export interface MultiStatOptionValueMap {
    [statId : string] : StatOptionValueMap
}

interface StatObjectData {
    id: string
    name: string
    graphql: GraphqlPathData
    options?: StatOption[]
    optionForX? : string // Which option to use to determine the type of the X axis for the stat.

    xPath: GraphqlSingleNodePath
    yPath: GraphqlSingleNodePath
}

interface StatLibraryFileData {
    id: string
    name: string
    includeStats?: string[]
    stats?: StatObjectData[]
    commonOptions?: StatOption[]
    optionForX? : string
}

// Holds a mapping of stat keys to stat objects which are what we need to
// send out a GraphQL request for stats. This is a definitive source of all the
// possible stats that a user could request and it should map to available stats in the
// GraphQL schema. Note that a single "stat" is made up of the stat in question as
// well as its accompanying attributes.
class StatLibrary {
    stats: StatObjectData[]
    statMap : Map<string, StatObjectData>

    exists(stat: string) : boolean {
        return this.statMap.has(stat)
    }

    get allStats() : string[] {
        return this.stats.map((ele: StatObjectData) => ele.id)
    }

    getStatOptions(stat: string) : StatOption[] | undefined {
        if (!this.exists(stat)) {
            return undefined
        }
        return this.statMap.get(stat)?.options
    }

    getStatName(stat: string) : string | undefined {
        if (!this.exists(stat)) {
            return undefined
        }
        return this.statMap.get(stat)?.name
    }

    getStatGraphqlPath(stat: string) : GraphqlPathData | undefined {
        if (!this.exists(stat)) {
            return undefined
        }
        return this.statMap.get(stat)?.graphql
    }

    getStatId(stat: string) : string | undefined {
        if (!this.exists(stat)) {
            return undefined
        }
        return this.statMap.get(stat)?.id
    }

    getStatOptionForX(stat : string) : string | undefined {
        if (!this.exists(stat)) {
            return undefined
        }
        return this.statMap.get(stat)?.optionForX
    }

    getStatXPath(stat : string, alias: GraphqlAlias | null = null) : GraphqlSingleNodePath | undefined {
        if (!this.exists(stat)) {
            return undefined
        }

        let path = [...this.statMap.get(stat)!.xPath]
        if (!!alias) {
            for (let i = 0; i < path.length; ++i) {
                if (path[i] == alias.original) {
                    path[i] = alias.alias
                }
            }
        }
        return path
    }

    getStatYPath(stat : string, alias: GraphqlAlias | null = null) : GraphqlSingleNodePath | undefined {
        if (!this.exists(stat)) {
            return undefined
        }

        let path = [...this.statMap.get(stat)!.yPath]
        if (!!alias) {
            for (let i = 0; i < path.length; ++i) {
                if (path[i] == alias.original) {
                    path[i] = alias.alias
                }
            }
        }
        return path
    }

    constructor(rawData : StatLibraryFileData, prefix: string = '', inPath: string = '', options: StatOption[] = [], optionForX: string | undefined = undefined) {
        let currentPrefix = (prefix.length > 0) ? `${prefix}_${rawData.id}` : rawData.id
        let currentPath = (inPath.length > 0) ? `${inPath}/${rawData.name}` : rawData.name
        let currentOptions = !!rawData.commonOptions ? [...rawData.commonOptions, ...options] : options
        let newOptionForX = !!rawData.optionForX ? rawData.optionForX : optionForX

        this.statMap = new Map<string, StatObjectData>()
        this.stats = []
        
        if (!!rawData.includeStats) {
            for (let inc of rawData.includeStats) {
                let finalInc = process.env.NODE_ENV === 'development' ? 
                    inc : path.join(process.resourcesPath, inc)
                let rawData = fs.readFileSync(finalInc, {encoding: 'utf-8'})
                let lib = new StatLibrary(JSON.parse(rawData), currentPrefix, currentPath, currentOptions, newOptionForX)
                this.stats.push(...lib.stats)
            }
        }

        if (!!rawData.stats) {
            this.stats.push(...rawData.stats.map((ele : StatObjectData) => {
                let ret = {...ele}
                ret.id = `${currentPrefix}_${ele.id}`
                ret.name = `${currentPath}/${ele.name}`
                ret.options = !!ret.options ? [...currentOptions, ...ret.options] : currentOptions
                ret.optionForX = !!ret.optionForX ? ret.optionForX : newOptionForX
                return ret
            }))
        }

        for (let st of this.stats) {
            this.statMap.set(st.id, st)
        }
    }
}

let rawStatData = fs.readFileSync(libFname, {encoding: 'utf-8'})
let library : StatLibrary
export default library = new StatLibrary(JSON.parse(rawStatData))