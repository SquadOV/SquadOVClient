<template>
    <div class="d-flex flex-column flex-grow-1">
        <div class="d-flex align-center mb-2">
            <span class="text-h5">{{ title }}</span>

            <v-dialog
                v-model="showHideAddStats"
                persistent
                max-width="40%"
            >
                <template v-slot:activator="{ on, attrs }">
                    <v-btn
                        icon
                        color="primary"
                        v-if="editable"
                        v-bind="attrs"
                        v-on="on"
                    >
                        <v-icon>
                            mdi-plus
                        </v-icon>
                    </v-btn>
                </template>
                
                <v-card>
                    <v-card-title>
                        Select Stats to Visualize
                    </v-card-title>
                    <v-divider></v-divider>

                    <stat-chooser
                        class="stat-container-div"
                        v-model="statsToAdd"
                        :available-stats="allAvailableStats"
                    >
                    </stat-chooser>

                    <v-card-actions>
                        <v-btn
                            @click="showHideAddStats = false"
                            color="error"
                        >
                            Cancel
                        </v-btn>

                        <v-spacer></v-spacer>

                        <v-btn
                            v-if="statsToAdd.length > 0"
                            color="success"
                            @click="addStats"
                        >
                            Add
                        </v-btn>
                    </v-card-actions>
                </v-card>
            </v-dialog>
            
        </div>
 
        <div
            class="d-flex align-center mb-4"
            v-for="(st, idx) in localStats"
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

            <v-btn icon
                color="error"
                @click="removeStat(st)"
                v-if="editable"
            >
                <v-icon>
                    mdi-delete
                </v-icon>
            </v-btn>
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
import StatChooser from '@client/vue/utility/stats/StatChooser.vue'

@Component({
    components: {
        StatOptionsChooser,
        StatChooser
    }
})
export default class StatContainer extends Vue {
    // These are the stats to pull and obtain series data for.
    // These stats will be made available to the slot display.
    @Prop({type: Array})
    stats! : string[]

    // We can't depend on a 'stats' array being passed in so the input 'stats'
    // array is just used as an initial state for the actual stats array, localStats.
    localStats: string[] = []
    statsToAdd: string[] = []

    @Prop({default: 'Graph'})
    title!: string

    @Prop({type: Boolean, default: false})
    editable!: boolean
    showHideAddStats : boolean = false

    seriesData: (StatXYSeriesData | null)[] = []
    statValues: MultiStatOptionValueMap | null = null

    get statSet() : Set<string> {
        return new Set<string>(this.localStats)
    }

    get allAvailableStats() : string[] {
        return StatLibrary.allStats.filter((ele : string) => {
            return !this.statSet.has(ele)
        })
    }

    addStats() {
        this.localStats.push(...this.statsToAdd)
        this.statsToAdd = []
        this.showHideAddStats = false
    }

    removeStat(st : string) {
        let idx = this.localStats.findIndex((ele: string) => ele == st)
        if (idx == -1) {
            return
        }
        this.localStats.splice(idx, 1)
    }

    @Watch('stats')
    syncToLocalStats() {
        if (!!this.stats) {
            this.localStats = [...this.stats]
        } else {
            this.localStats = []
        }
    }

    get validStats() : string[] {
        return this.localStats.filter((st :string) => {
            return StatLibrary.exists(st)
        })
    }

    getStateName(st : string) : string {
        if (!StatLibrary.exists(st)) {
            return 'Unknown'
        }
        return StatLibrary.getStatName(st)!
    }
    
    @Watch('localStats')
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
                        defaultValue = val
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
        if (!this.statValues || this.validStats.length == 0) {
            this.seriesData = []
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

                let id = StatLibrary.getStatId(st)!
                let optionValues = this.statValues![id]

                const xPath = StatLibrary.getStatXPath(st, alias)!
                const yPath = StatLibrary.getStatYPath(st, alias)!
                return loadStatXYSeriesDataFromGraphql(
                    resp.data.data,
                    xPath,
                    yPath,
                    optionValues.values[StatLibrary.getStatOptionForX(st)!].type!,
                    StatLibrary.getStatName(st)!,
                )
            })
        }).catch((err: any) => {
            console.log('Failed to perform GraphQL query: ', err)
        })
    }

    mounted() {
        this.syncToLocalStats()
        this.regenerateStatOptions()
    }
}

</script>

<style scoped>

.stat-container-div {
    max-height: 70vh;
    overflow-y: auto;
}

</style>