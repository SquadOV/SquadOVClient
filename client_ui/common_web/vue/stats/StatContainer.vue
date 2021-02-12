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

interface BoxedStatInstance {
    stat: string
    instance: StatInstance | null
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

    @Prop({required: true})
    userId!: number

    // These are the stats to pull and obtain series data for.
    // These stats will be made available to the slot display.
    @Prop({type: Array, default: () => []})
    stats! : string[]
    statsToAdd: string[] = []
    showHideAddStats: boolean = false

    // This is the final set of stats to visualize. This is 
    // separate from the input stats prop array as we may add
    // user-specified stats on top of that.
    statInstances: BoxedStatInstance[] = []

    seriesData:  (StatXYSeriesData | undefined)[] = []

    get allAvailableStats() : string[] {
        return statLibrary.allStatIds
    }

    @Watch('stats')
    refreshStatSet() {
        this.statInstances = this.stats.map((ele: string) => ({
            stat: ele,
            instance: null
        }))
    }


    addStats() {
        this.statInstances.push(...this.statsToAdd.map((ele: string) => ({
            stat: ele,
            instance: null
        })))
        this.statsToAdd = []
        this.showHideAddStats = false
    }

    removeStat(stat: StatInstance) {
        this.statInstances = this.statInstances.filter((ele: BoxedStatInstance) => ele.instance?.uuid !== stat.uuid)
    }

    @Watch('statInstances')
    refreshStatInstances() {
        // We only need to create instances for the stats that don't already exist.
        // This way we can retain options between pulls.
        this.statInstances.forEach((ele: BoxedStatInstance, idx: number) => {
            if (!!ele.instance) {
                return
            }
        
            let inst = statLibrary.createInstance(ele.stat)
            inst.name = `${inst.name} ${idx}`
            Vue.set(ele, 'instance', inst)
        })
    }

    get instances(): StatInstance[] {
        return this.statInstances.filter((ele: BoxedStatInstance) => !!ele.instance).map((ele: BoxedStatInstance) => {
            return ele.instance!
        })
    }

    @Watch('instances', {deep:true})
    pullGraphqlData() {
        if (this.instances.length == 0) {
            this.seriesData = []
            return
        }

        // TODO: Figure out *what* changed exactly so we can only request data that we expect to be different.
        let container = new StatInstanceContainer(new Map([[this.userId, this.instances]]))
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