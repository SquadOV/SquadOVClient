import { GraphqlAimlabGenericStatsQueryBuilder, GraphqlAimlabCommonParams } from '@client/js/graphql/aimlab/common'

export class GraphqlAimlabSparStatsQueryBuilder<T> extends GraphqlAimlabGenericStatsQueryBuilder<T> {
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