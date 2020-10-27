import { BuilderNode } from '@client/js/graphql/builderPrimitives'
import { StatGroupFunctions, StatSortDirection} from '@client/js/stats/statPrimitives'
import { AimlabGrouping } from '@client/js/stats/aimlabStatLibrary'

export interface GraphqlAimlabCommonParams {
    group: AimlabGrouping
    func: StatGroupFunctions
    sort: StatSortDirection
}

export class GraphqlAimlabGenericStatsQueryBuilder<T> extends BuilderNode<T> {
    constructor(p : T, node: string, params: GraphqlAimlabCommonParams) {
        super(p, node)
        this.addAttributes([
            'date',
            'datetime',
            'time',
            'version',
            'id'
        ])

        this.addParam('params', `{ group:${params.group}, func:${params.func}, sort:${params.sort} }`)
    }

    score() : this {
        this.addAttributes(['score'])
        return this
    }
}