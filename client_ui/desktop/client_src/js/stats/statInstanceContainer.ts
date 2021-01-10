import { StatInstance } from '@client/js/stats/statPrimitives'
import { GraphqlApiData } from '@client/js/api' 
import { GraphqlQueryBuilder } from '@client/js/graphql/graphql'
import { AllStats, cleanAllStatsFromJson } from '@client/js/graphql/schema'

function buildGraphqlAlias(userUuid: string, instanceUuid: string): string {
    return `user${userUuid.replace(/-/g, '')}_stat${instanceUuid.replace(/-/g, '')}`
}

export class StatInstanceContainer {
    private _stats: Map<string, StatInstance[]>

    constructor(statsPerUser: Map<string, StatInstance[]>) {
        this._stats = statsPerUser
    }

    buildGraphqlQuery() : string {
        let builder = new GraphqlQueryBuilder()
        for (let [uuid, inst] of this._stats) {
            for (let st of inst) {
                let sb = builder.stats(buildGraphqlAlias(uuid, st.uuid), `"${uuid}"`)
                st.obj.addToGraphqlBuilder(sb, st.options)
            }
        }
        return builder.build()
    }

    parseGraphqlResponse(data: GraphqlApiData<any>) {
        // yikes
        let response = data.data.data
        if (!response) {
            return
        }

        for (let [uuid, inst] of this._stats) {
            for (let st of inst) {
                let userStats: AllStats = response[buildGraphqlAlias(uuid, st.uuid)]
                cleanAllStatsFromJson(userStats)
                st.obj.parseResponseIntoInstance(userStats, st)
            }
        }
    }
}