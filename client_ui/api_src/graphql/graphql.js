const { graphqlHTTP } = require('express-graphql')
const { makeExecutableSchema } = require('@graphql-tools/schema');
const { mergeResolvers } = require('@graphql-tools/merge');
const { resolvers: StatsResolver  } = require('./stats.js')
const { resolvers: AimlabStatsResolver } = require('./stats/aimlab.js')
const fs = require('fs')

const schemaFiles = [
    'graphql/query.graphql',
    'graphql/stats/stats.graphql',
    'graphql/stats/aimlab_stats.graphql'
]

module.exports.createGraphqlEndpoint = function(db) {
    let fullSchemaData = []
    for (let f of schemaFiles) {
        let data = fs.readFileSync(f)
        fullSchemaData.push(data)
    }

    return graphqlHTTP({
        schema: makeExecutableSchema({
            typeDefs: fullSchemaData.join('\n'),
            resolvers: mergeResolvers([
                StatsResolver,
                AimlabStatsResolver,
            ]),
        }),
        context: { db: db },
        graphiql: true,
    })
}