<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <div class="mr-1">
                Data:
            </div>
            <date-range-picker
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            >
            </date-range-picker>

            <div class="mx-1">
                Cohort:
            </div>
            <date-range-picker
                :start-date.sync="cohortStartDate"
                :end-date.sync="cohortEndDate"
            >
            </date-range-picker>

            <v-btn icon @click="downloadData">
                <v-icon>
                    mdi-download
                </v-icon>
            </v-btn>

            <v-select
                v-model="mode"
                :items="modeItems"
                label="Action"
                hide-details
            >
            </v-select>
        </div>
        <v-divider></v-divider>

        <v-row>
            <v-col cols="12">
                <div ref="graphDiv" style="width: 100%; height: calc(100vh - 144px);"></div>
            </v-col>
        </v-row>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import { Watch } from 'vue-property-decorator'
import { getPowerUserCurve } from '@client/ts/poweruser'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/bar'
import 'echarts/lib/component/grid'
import 'echarts/lib/component/tooltip'

@Component({
    components: {
        DateRangePicker,
    }
})
export default class PowerUserCurve extends Vue {
    startDate: Date = new Date()
    endDate: Date = new Date()

    cohortStartDate: Date = new Date()
    cohortEndDate: Date = new Date()
    mode: number = 0

    data: number[] = []
    graph : any | null = null

    $refs!: {
        graphDiv: HTMLElement
    }

    get modeItems(): any[] {
        return [
            {
                text: 'Daily',
                value: 0,
            },
            {
                text: 'Weekly',
                value: 1,
            },
        ]
    }

    @Watch('startDate')
    @Watch('endDate')
    @Watch('cohortStartDate')
    @Watch('cohortEndDate')
    @Watch('mode')
    refreshData() {
        this.data = []
        
        getPowerUserCurve(this.startDate, this.endDate, this.cohortStartDate, this.cohortEndDate, this.mode).then((resp: number[]) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get power user curve: ', err)
        })
    }

    @Watch('data')
    refreshGraph() {
        if (!this.graph || this.data.length == 0) {
            return
        }

        let options : any = {
            xAxis: {
                type: 'category',
                data: this.data.map((_val: number, idx: number) => `${idx+1}`),
            },
            yAxis: {
                type: 'value',
            },
            tooltip: {
                trigger: 'axis',
            },
            series: [
                {
                    data: this.data,
                    type: 'bar',
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

    downloadData() {
        let content = 'data:text/csv;charset=utf-8,'
        content += ['Days Active', 'Count'].join(',')
        content += '\r\n'
        content += this.data.map((ele: any, idx: number) => [
            idx + 1,
            ele,
        ].join(',')).join('\r\n')

        let encoded = encodeURI(content)
        window.open(encoded)
    }
}

</script>