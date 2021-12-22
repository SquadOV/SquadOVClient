<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <date-range-picker
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            >
            </date-range-picker>
        </div>
        <v-divider></v-divider>

        <v-row>
            <v-col cols="12">
                <div ref="graphDiv" style="width: 100%; height: calc(100vh - 60px);"></div>
            </v-col>
        </v-row>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { getReferralFlow, ReferralFlowData } from '@client/ts/referrals'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/sankey'
import 'echarts/lib/component/grid'
import 'echarts/lib/component/tooltip'

@Component({
    components: {
        DateRangePicker,
    }
})
export default class ReferralFlow extends Vue {
    startDate: Date = new Date()
    endDate: Date = new Date()
    graph : any | null = null
    data: ReferralFlowData | null = null

    $refs!: {
        graphDiv: HTMLElement
    }

    @Watch('startDate')
    @Watch('endDate')
    refreshData() {
        this.data = null
        
        getReferralFlow(this.startDate, this.endDate).then((resp: ReferralFlowData) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get referral flow data: ', err)
        })
    }

    @Watch('data')
    refreshGraph() {
        if (!this.graph || !this.data) {
            return
        }

        let options : any = {
            tooltip: {
                trigger: 'item',
                triggerOn: 'mousemove'
            },
            series: [
                {
                    data: this.data.steps.map((ele: string) => {
                        return {
                            name: ele,
                        }
                    }),
                    type: 'sankey',
                    emphasiss: {
                        focus: 'adjacency'
                    },
                    links: this.data.links
                }
            ]
        }

        this.graph.setOption(options)
    }

    mounted() {
        this.refreshData()
        this.graph = echarts.init(this.$refs.graphDiv, null, { renderer: 'canvas' })
        window.addEventListener('resize', this.graphResize)
    }

    graphResize() {
        if (!!this.graph) {
            this.graph.resize()
        }
    }

    beforeDestroy() {
        window.removeEventListener('resize', this.graphResize)
    }

}

</script>