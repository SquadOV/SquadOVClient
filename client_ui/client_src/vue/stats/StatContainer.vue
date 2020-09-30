<template>
    <div class="pa-4">
        <div class="mb-2">
            <span class="text-h5">{{ title }}</span>
        </div>
 
        <div
            class="d-flex align-center"
            v-for="(st, idx) in stats"
            :key="idx"
        >
            <span
                class="text-subtitle mr-4"
            >
                {{ getStateName(st) }}
            </span>

            <stat-options-chooser
                class="flex-grow-1"
                v-if="!!statValues"
                :option-values="statValues[st]"
            >
            </stat-options-chooser>
        </div>

        <slot v-bind:data="seriesData"></slot>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { GraphqlQuery, GraphqlAlias } from '@client/js/graphql/graphql'
import { GraphqlQuerySchema } from '@client/js/graphql/schema'
import { apiClient, GraphqlApiData } from '@client/js/api'
import StatLibrary, { MultiStatOptionValueMap, StatOptionValueMap, createGraphqlVariables } from '@client/js/stats/stat_library'
import { loadStatXYSeriesDataFromGraphql, StatXYSeriesData } from '@client/js/stats/series_data'

import StatOptionsChooser from '@client/vue/stats/StatOptionsChooser.vue'

@Component({
    components: {
        StatOptionsChooser
    }
})
export default class StatContainer extends Vue {
    // These are the stats to pull and obtain series data for.
    // These stats will be made available to the slot display.
    @Prop({type: Array})
    stats! : string[]

    @Prop({default: 'Graph'})
    title!: string

    seriesData: (StatXYSeriesData | null)[] = []
    statValues: MultiStatOptionValueMap | null = null

    get validStats() : string[] {
        return this.stats.filter((st :string) => {
            return StatLibrary.exists(st)
        })
    }

    getStateName(st : string) : string {
        if (!StatLibrary.exists(st)) {
            return 'Unknown'
        }
        return StatLibrary.getStatName(st)!
    }
    
    @Watch('stats')
    regenerateStatOptions() {
        let options : MultiStatOptionValueMap = {}

        for (let st of this.validStats) {
            let statOptions = StatLibrary.getStatOptions(st)!
            let map : StatOptionValueMap = {
                options: statOptions,
                values: {},
            }

            // Populate the map with default values.
            for (let opt of statOptions) {
                let defaultValue : any = null
                for (let val of opt.values) {
                    if (!!val.default) {
                        defaultValue = val.value
                    }
                }
                map.values[opt.id] = defaultValue
            }

            options[st] = map
        }

        this.statValues = options
    }

    @Watch('statValues', {deep: true})
    refreshStats() {
        if (!this.statValues) {
            return
        }

        let query = new GraphqlQuery()
        for (let st of this.validStats) {
            let id = StatLibrary.getStatId(st)!
            query.addPath(id, StatLibrary.getStatGraphqlPath(st)!)
            query.addVariables(id, createGraphqlVariables(this.statValues[id]))
        }

        apiClient.graphqlRequest(query).then((resp : GraphqlApiData<any>) => {
            this.seriesData = this.validStats.map((st: string): StatXYSeriesData | null => {
                const alias: GraphqlAlias = {
                    alias: StatLibrary.getStatId(st)!,
                    original: 'stats'
                }
                const xPath = StatLibrary.getStatXPath(st, alias)!
                const yPath = StatLibrary.getStatYPath(st, alias)!
                return loadStatXYSeriesDataFromGraphql(resp.data.data, xPath, yPath)
            })
        }).catch((err: any) => {
            console.log('Failed to perform GraphQL query: ', err)
        })
    }

    mounted() {
        this.regenerateStatOptions()
    }
}

</script>

<style scoped>

</style>