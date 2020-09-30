type StatGraphqlVarabiableMap = Map<string, GraphqlVariable>

export class GraphqlQuery {
    paths : Map<string, GraphqlPathData>
    vars : Map<string, Map<string, GraphqlVariable>>

    constructor() {
        this.paths = new Map<string, GraphqlPathData>()
        this.vars = new Map<string, StatGraphqlVarabiableMap>()
    }

    addPath(alias : string, path : GraphqlPathData) {
        this.paths.set(alias, path)
    }

    addVariables(id : string, vars : GraphqlVariable[]) {
        let map : StatGraphqlVarabiableMap = new Map()
        for (let v of vars) {
            map.set(v.key, v.value)
        }
        this.vars.set(id, map)
    }

    generateBody() : string {
        let body: string[] = []
        this.paths.forEach((value, key) => {
            body.push(`${key}: ${generateGraphqlForPath(value, this.vars.get(key)!)}`)
        })
        return `{ ${body.join('\n')} }`
    }
}

export type GraphqlSingleNodePath = string[]

export interface GraphqlPathData {
    node: string
    args?: {
        key: string
        value: any
        isEnum?: boolean
        isVariable?: boolean
    }[]
    children?: GraphqlPathData[]
}

export interface GraphqlAlias {
    original: string
    alias: string
}

export interface GraphqlVariable {
    key: string
    value: any
}

function generateGraphqlForPath(path : GraphqlPathData, vars : StatGraphqlVarabiableMap) : string {
    let txt = `${path.node}`

    if (!!path.args) {
        let args : string[] = []
        for (let a of path.args) {
            let val : any = a.value
            if (!!a.isVariable) {
                val = vars.get(val)!
            }

            if (typeof val === 'string' && !a.isEnum) {
                args.push(`${a.key}: "${val}"`)
            } else {
                args.push(`${a.key}: ${val}`)
            }
        }

        txt = `${txt} (${args.join(', ')})`
    }

    if (!path.children || path.children.length == 0) {
        return txt
    }

    let childNodes: string[] = path.children.map((ele : GraphqlPathData) => {
        return generateGraphqlForPath(ele, vars)
    })
    return `${txt} { ${childNodes.join('\n') } }`
}