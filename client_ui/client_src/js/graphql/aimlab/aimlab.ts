import { BuilderNode } from '@client/js/graphql/builderPrimitives'
import { GraphqlAimlabGenericStatsQueryBuilder, GraphqlAimlabCommonParams } from '@client/js/graphql/aimlab/common'
import {
    GraphqlAimlabSparStatsQueryBuilder
} from '@client/js/graphql/aimlab/spar'

type GenModeFuncBuilder = <T>(parent: GraphqlAimlabTaskModeStatsQueryBuilder<T>, mode: string, params: GraphqlAimlabCommonParams) => any

class GraphqlAimlabTaskModeStatsQueryBuilder<T> extends BuilderNode<T> {
    private _modeBuilder: GenModeFuncBuilder

    constructor(p : T, task: string, modeBuilder: GenModeFuncBuilder) {
        super(p, task)
        this._modeBuilder = modeBuilder
    }

    // Not sure how much it matters but it'd be nice to have these
    // functions return the actual type instead of 'any'.

    ultimate(params: GraphqlAimlabCommonParams) : any {
        let b = this._modeBuilder<T>(this, 'ultimate', params)
        this.addAliasedChild(b)
        return b
    }
    
    speed(params: GraphqlAimlabCommonParams) : any {
        let b = this._modeBuilder<T>(this, 'speed', params)
        this.addAliasedChild(b)
        return b
    }

    precision(params: GraphqlAimlabCommonParams) : any {
        let b = this._modeBuilder<T>(this, 'precision', params)
        this.addAliasedChild(b)
        return b
    }
}

export class GraphqlAimlabStatsQueryBuilder<T> extends BuilderNode<T> {
    constructor(p : T) {
        super(p, 'aimlab')
    }

    createTaskBuilder<U>(task : string, func : GenModeFuncBuilder) : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        return new GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>>(
            this,
            task,
            func
        )
    }

    gridshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('gridshot', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    spidershot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('spidershot', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    microshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('microshot', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    sixshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('sixshot', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    microflex() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('microflex', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    motionshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('motionshot', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    multishot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('multishot', createAimlabSparStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    detection() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('detection', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    decisionshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('decisionshot', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    strafetrack() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('strafetrack', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    circletrack() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('circletrack', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    strafeshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('strafeshot', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    circleshot() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('circleshot', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    linetrace() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('linetrace', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    multilinetrace() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('multilinetrace', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }

    pentakill() : GraphqlAimlabTaskModeStatsQueryBuilder<GraphqlAimlabStatsQueryBuilder<T>> {
        let b = this.createTaskBuilder('pentakill', createAimlabCommonStatsBuilder)
        this.addAliasedChild(b)
        return b
    }
}

export function createAimlabSparStatsBuilder<T>(parent: GraphqlAimlabTaskModeStatsQueryBuilder<T>, mode: string, params: GraphqlAimlabCommonParams) {
    return new GraphqlAimlabSparStatsQueryBuilder<GraphqlAimlabTaskModeStatsQueryBuilder<T>>(parent, mode, params)
}

export function createAimlabCommonStatsBuilder<T>(parent: GraphqlAimlabTaskModeStatsQueryBuilder<T>, mode: string, params: GraphqlAimlabCommonParams) {
    return new GraphqlAimlabGenericStatsQueryBuilder<GraphqlAimlabTaskModeStatsQueryBuilder<T>>(parent, mode, params)
}