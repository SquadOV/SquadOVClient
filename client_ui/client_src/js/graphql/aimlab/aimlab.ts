import { BuilderNode } from '@client/js/graphql/builderPrimitives'
import {
    GraphqlAimlabGridshotStatsQueryBuilder
} from '@client/js/graphql/aimlab/spar'

export class GraphqlAimlabStatsQueryBuilder<T> extends BuilderNode<T> {
    constructor(p : T) {
        super(p, 'aimlab')
    }

    gridshot() : GraphqlAimlabGridshotStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = new GraphqlAimlabGridshotStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>>(this)
        this.addAliasedChild(b)
        return b
    }
}