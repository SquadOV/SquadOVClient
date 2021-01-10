import { BuilderNode, BaseNode } from '@client/js/graphql/builderPrimitives'
import { GraphqlAimlabStatsQueryBuilder } from '@client/js/graphql/aimlab/aimlab'

export class GraphqlStatsQueryBuilder<T> extends BuilderNode<T> {
    constructor(p : T, userUuid : string) {
        super(p, 'stats')
        this.addParam('userUuid', userUuid)
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

    stats(alias: string, uuid: string) : GraphqlStatsQueryBuilder<GraphqlQueryBuilder> {
        let b = new GraphqlStatsQueryBuilder(this, uuid)
        this.addAliasedChild(b, alias)
        return b
    }
}