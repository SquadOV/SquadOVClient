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
            v-for="(st, idx) in instances"
            :key="idx"
        >
            <span
                class="text-subtitle mr-4"
            >
                {{ st.name }}
            </span>

            <stat-option-chooser
                class="flex-grow-1"
                :option="st.options"
            >
            </stat-option-chooser>

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
import { apiClient, GraphqlApiData } from '@client/js/api'
import { statLibrary } from '@client/js/stats/statLibrary'
import { StatInstance } from '@client/js/stats/statPrimitives'
import { StatInstanceContainer } from '@client/js/stats/statInstanceContainer'
import { StatXYSeriesData } from '@client/js/stats/seriesData'
import StatChooser from '@client/vue/stats/StatChooser.vue'
import StatOptionChooser from '@client/vue/stats/StatOptionChooser.vue'

interface UniqueStatId {
    stat: string
    id : number
    alias: string
}

@Component({
    components: {
        StatOptionChooser,
        StatChooser
    }
})
export default class StatContainer extends Vue {
    @Prop({default: 'Graph'})
    title!: string

    @Prop({type: Boolean, default: false})
    editable!: boolean

    // These are the stats to pull and obtain series data for.
    // These stats will be made available to the slot display.
    @Prop({type: Array})
    stats! : string[]
    statsToAdd: string[] = []
    showHideAddStats: boolean = false

    // This is the final set of stats to visualize. This is 
    // separate from the input stats prop array as we may add
    // user-specified stats on top of that.
    finalStats: string[] = []
    instances: StatInstance[] = []
    seriesData:  (StatXYSeriesData | undefined)[] = []

    get allAvailableStats() : string[] {
        return statLibrary.allStatIds
    }

    @Watch('stats')
    refreshStatSet() {
        this.finalStats = Array.from(new Set([...this.stats]))
    }

    get statSet() : Set<string> {
        return new Set([...this.finalStats])
    }

    addStats() {
        for (let s of this.statsToAdd) {
            if (!this.statSet.has(s)) {
                this.finalStats.push(s)
            }
        }
        this.statsToAdd = []
        this.showHideAddStats = false
    }

    removeStat(stat: StatInstance) {
        this.instances = this.instances.filter((ele: StatInstance) => ele.id != stat.id)
    }

    @Watch('finalStats')
    refreshStatInstances() {
        let existingInstances = this.instances.filter((ele : StatInstance) => this.statSet.has(ele.id))
        let newInstances = new Map<string, StatInstance>(existingInstances.map((ele : StatInstance) => [ele.id, ele]))

        // We only need to create instances for the stats that don't already exist.
        // This way we can retain options between pulls.
        let newStats : StatInstance[] = []
        for (let stat of this.finalStats) {
            if (newInstances.has(stat)) {
                continue
            }
            
            let inst = statLibrary.createInstance(stat)
            newStats.push(inst)
            newInstances.set(inst.id, inst)
        }

        this.instances = this.finalStats.map((ele : string) => newInstances.get(ele)!)
    }

    @Watch('instances', {deep:true})
    pullGraphqlData() {
        if (this.instances.length == 0) {
            return
        }

        // TODO: Figure out *what* changed exactly so we can only request data that we expect to be different.
        let container = new StatInstanceContainer(new Map([[this.$store.state.currentUser.uuid, this.instances]]))
        let request = container.buildGraphqlQuery()
        apiClient.graphqlRequest(request).then((resp : GraphqlApiData<any>) => {
            container.parseGraphqlResponse(resp)
            this.seriesData = this.instances.map((ele : StatInstance) => ele.data)
        }).catch((err: any) => {
            console.log('Failed to obtain GraphQL data: ', err)
        })
    }

    mounted() {
        this.refreshStatSet()
    }
}

</script>

<style scoped>

.stat-container-div {
    max-height: 70vh;
    overflow-y: auto;
}

</style>