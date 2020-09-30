const libFname = 'assets/stats/library.json'
import fs from 'fs'
import { GraphqlPathData, GraphqlSingleNodePath, GraphqlAlias, GraphqlVariable } from '@client/js/graphql/graphql'

export interface StatOption {
    id: string
    name: string
    variable: string
    isEnum? : boolean
    values: {
        text: string
        value: any
        default?: boolean
    }[]
}

export interface StatOptionValueMap {
    options: StatOption[]
    values: {[ optId : string ] : any}
}

export function createGraphqlVariables(map : StatOptionValueMap) : GraphqlVariable[] {
    return map.options.map((ele : StatOption) => {
        return {
            key: ele.variable,
            value: map.values[ele.id],
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

    xPath: GraphqlSingleNodePath
    yPath: GraphqlSingleNodePath
}

interface StatLibraryFileData {
    id: string
    name: string
    includeStats?: string[]
    stats?: StatObjectData[]
    commonOptions?: StatOption[]
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

    getStatXPath(stat : string, alias: GraphqlAlias | null = null) : GraphqlSingleNodePath | undefined {
        if (!this.exists(stat)) {
            return undefined
        }

        let path = this.statMap.get(stat)!.xPath
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

        let path = this.statMap.get(stat)!.yPath
        if (!!alias) {
            for (let i = 0; i < path.length; ++i) {
                if (path[i] == alias.original) {
                    path[i] = alias.alias
                }
            }
        }
        return path
    }

    constructor(rawData : StatLibraryFileData, prefix: string = '', path: string = '', options: StatOption[] = []) {
        let currentPrefix = (prefix.length > 0) ? `${prefix}_${rawData.id}` : rawData.id
        let currentPath = (path.length > 0) ? `${path}/${rawData.name}` : rawData.name
        let currentOptions = !!rawData.commonOptions ? [...rawData.commonOptions, ...options] : options

        this.statMap = new Map<string, StatObjectData>()
        this.stats = []
        
        if (!!rawData.includeStats) {
            for (let inc of rawData.includeStats) {
                let rawData = fs.readFileSync(inc, {encoding: 'utf-8'})
                let lib = new StatLibrary(JSON.parse(rawData), currentPrefix, currentPath, currentOptions)
                this.stats.push(...lib.stats)
            }
        }

        if (!!rawData.stats) {
            this.stats.push(...rawData.stats.map((ele : StatObjectData) => {
                let ret = {...ele}
                ret.id = `${currentPrefix}_${ele.id}`
                ret.name = `${currentPath}/${ele.name}`
                ret.options = !!ret.options ? [...currentOptions, ...ret.options] : currentOptions
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