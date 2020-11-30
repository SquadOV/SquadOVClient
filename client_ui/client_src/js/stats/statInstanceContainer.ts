import { StatInstance } from '@client/js/stats/statPrimitives'
import { GraphqlApiData } from '@client/js/api' 
import { GraphqlQueryBuilder } from '@client/js/graphql/graphql'
import { AllStats, cleanAllStatsFromJson } from '@client/js/graphql/schema'

export class StatInstanceContainer {
    private _stats: Map<string, StatInstance[]>

    constructor(statsPerUser: Map<string, StatInstance[]>) {
        this._stats = statsPerUser
    }

    buildGraphqlQuery() : string {
        let builder = new GraphqlQueryBuilder()
        for (let [uuid, inst] of this._stats) {
            let sb = builder.stats(`user${uuid.replace(/-/g, '')}`, `"${uuid}"`)
            for (let st of inst) {
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
            let userStats: AllStats = response[`user${uuid.replace(/-/g, '')}`]
            cleanAllStatsFromJson(userStats)

            for (let st of inst) {
                st.obj.parseResponseIntoInstance(userStats, st)
            }
        }
    }
}