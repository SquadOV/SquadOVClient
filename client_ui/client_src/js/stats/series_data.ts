import { GraphqlSingleNodePath } from '@client/js/graphql/graphql'
import { assert } from 'console'

export class StatXYSeriesData {
    _x : any[]
    _y : any[]

    constructor(x : any[], y : any[]) {
        this._x = x
        this._y = y
    }
}

function loadSingleDataFromGraphqlPath(stats : any, path : GraphqlSingleNodePath) : any {
    let node = path[0]
    
    if (!(node in stats)) {
        return null
    }

    let val = stats[node]
    if (path.length == 1) {
        return val
    } else {
        return loadSingleDataFromGraphqlPath(val, path.slice(1))
    }
}

function loadDataFromGraphqlPath(stats: any, path : GraphqlSingleNodePath) : any[] {
    if (path.length == 0) {
        return []
    }

    if (!Array.isArray(stats)) {
        let node = path[0]
        if (!(node in stats)) {
            return []
        }

        return loadDataFromGraphqlPath(stats[node], path.slice(1))
    } else {
        return stats.map((ele : any) => {
            return loadSingleDataFromGraphqlPath(ele, path)
        })
    }
}

export function loadStatXYSeriesDataFromGraphql(stats: any, xPath: GraphqlSingleNodePath, yPath: GraphqlSingleNodePath) : StatXYSeriesData {
    let x = loadDataFromGraphqlPath(stats, xPath)
    let y = loadDataFromGraphqlPath(stats, yPath)
    assert(x.length == y.length)

    return new StatXYSeriesData(x, y)
}