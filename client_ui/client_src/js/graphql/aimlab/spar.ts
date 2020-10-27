import { GraphqlAimlabGenericStatsQueryBuilder, GraphqlAimlabCommonParams } from '@client/js/graphql/aimlab/common'
import { BuilderNode } from '@client/js/graphql/builderPrimitives'

class GraphqlAimlabSparStatsQueryBuilder<T> extends GraphqlAimlabGenericStatsQueryBuilder<T> {
    constructor(p : T, node: string, params: GraphqlAimlabCommonParams) {
        super(p, node, params)
    }

    kill() : this {
        this.addAttributes(['kill'])
        return this
    }

    ttk() : this {
        this.addAttributes(['ttk'])
        return this
    }

    acc() : this {
        this.addAttributes(['acc'])
        return this
    }
}

export class GraphqlAimlabGridshotStatsQueryBuilder<T> extends BuilderNode<T> {
    constructor(p: T) {
        super(p, 'gridshot')
    }

    ultimate(params: GraphqlAimlabCommonParams): GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabGridshotStatsQueryBuilder<T>> {
        let b = new GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabGridshotStatsQueryBuilder<T>>(this, 'ultimate', params)
        this.addAliasedChild(b)
        return b
    }

    precision(params: GraphqlAimlabCommonParams): GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabGridshotStatsQueryBuilder<T>> {
        let b = new GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabGridshotStatsQueryBuilder<T>>(this, 'precision', params)
        this.addAliasedChild(b)
        return b
    }

    speed(params: GraphqlAimlabCommonParams): GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabGridshotStatsQueryBuilder<T>> {
        let b = new GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabGridshotStatsQueryBuilder<T>>(this, 'speed', params)
        this.addAliasedChild(b)
        return b
    }
}