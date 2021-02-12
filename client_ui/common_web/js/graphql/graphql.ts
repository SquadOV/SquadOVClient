import { BuilderNode, BaseNode } from '@client/js/graphql/builderPrimitives'
import { GraphqlAimlabStatsQueryBuilder } from '@client/js/graphql/aimlab/aimlab'

export class GraphqlStatsQueryBuilder<T> extends BuilderNode<T> {
    constructor(p : T, userId : number) {
        super(p, 'stats')
        this.addParam('userId', `"${userId}"`)
    }

    aimlab() : GraphqlAimlabStatsQueryBuilder<GraphqlStatsQueryBuilder<T>> {
        let b = new GraphqlAimlabStatsQueryBuilder<GraphqlStatsQueryBuilder<T>>(this)
        this.addAliasedChild(b)
        return b
    }
}

export class GraphqlQueryBuilder extends BaseNode {
    constructor() {
        super('')
    }

    stats(alias: string, id: number) : GraphqlStatsQueryBuilder<GraphqlQueryBuilder> {
        let b = new GraphqlStatsQueryBuilder(this, id)
        this.addAliasedChild(b, alias)
        return b
    }
}