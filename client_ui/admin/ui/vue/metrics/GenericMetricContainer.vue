<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <h1>{{ title }}</h1>
            <v-spacer></v-spacer>
            <time-interval-select v-model="interval" class="mr-4"></time-interval-select>
            <date-range-picker
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            ></date-range-picker>
        </div>
        <v-divider></v-divider>

        <template>
            <v-row>
                <v-col cols="6">
                    <v-data-table
                        v-if="!!data"
                        :headers="tableHeaders"
                        :items="tableData"
                        :items-per-page="30"
                    >
                    </v-data-table>
                    
                    <div class="d-flex justify-center" v-else>
                        <v-progress-circular size="64" indeterminate></v-progress-circular>
                    </div>
                </v-col>

                <v-col cols="6">
                    <div ref="graphDiv" style="width: 100%; height: calc(100vh - 120px);"></div>
                </v-col>
            </v-row>
        </template>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { Interval } from '@client/ts/interval'
import { Metrics, MetricDatum, getMetricData } from '@client/ts/metrics'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import TimeIntervalSelect from '@client/vue/TimeIntervalSelect.vue'
import format from 'date-fns/format'

import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/line'
import 'echarts/lib/component/axisPointer'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/grid'
import 'echarts/lib/component/legend'
import 'echarts/lib/component/legendScroll'
import 'echarts/lib/component/tooltip'
import 'echarts/lib/component/markLine'
import 'echarts/lib/component/markArea'
import 'echarts/lib/component/visualMap'

@Component({
    components: {
        DateRangePicker,
        TimeIntervalSelect,
    }
})
export default class GenericMetricContainer extends Vue {
    @Prop({required: true})
    title!: string

    @Prop({required: true})
    metric!: Metrics

    $refs!: {
        graphDiv: HTMLElement
    }

    interval: Interval = Interval.Daily
    startDate: Date = new Date()
    endDate: Date = new Date()

    get range(): Date[] {
        return [this.startDate, this.endDate]
    }

    data: MetricDatum[] | null = null
    graph : any | null = null

    @Watch('metric')
    @Watch('interval')
    @Watch('startDate')
    @Watch('endDate')
    refreshData() {
        this.data = null

        getMetricData(this.metric, this.interval, this.startDate, this.endDate).then((resp: MetricDatum[]) => {
            this.data = resp
        }).catch((err: any) => {
            console.log('Failed to get metric data: ', err)
        })
    }

    mounted() {
        this.refreshData()
        this.graph = echarts.init(this.$refs.graphDiv, null, { renderer: 'canvas' })
        window.addEventListener('resize', this.graphResize)
    }

    get tableHeaders(): any[] {
        if (!this.data || this.data.length == 0) {
            return []
        }

        let sub = this.data[0].sub
        return [{
            text: 'Time',
            value: 'tm',
        }, ...sub.map((ele: string) => ({
            text: ele,
            value: ele,
        }))]
    }

    get tableData(): any[] {
        if (!this.data || this.data.length == 0) {
            return []
        }

        return this.data.map((ele: MetricDatum) => ({
            tm: format(ele.tm, 'MM/dd/yyyy'),
            ...ele.data,
        }))
    }

    @Watch('data')
    refreshGraph() {
        if (!this.graph || !this.data  || this.data.length == 0) {
            return
        }

        let dataZooms: any[] = [
            {
                type: 'slider',
                show: true,
                textStyle: {
                    color: '#FFFFFF'
                }
            },
        ]

        let sub = this.data[0].sub
        let options : any = {
            legend: {
                show: true,
                type: 'scroll',
                textStyle: {
                    color: '#FFFFFF',
                },
            },
            xAxis: {
                type: 'category',
            },
            yAxis: {
                type: 'value'
            },
            axisPointer: {
                link: {xAxisIndex: 'all'}
            },
            dataZoom: dataZooms,
            tooltip: {
                trigger: 'axis',
            },
            series: sub.map((s: string) => {
                return {
                    data: this.data!.map((e: MetricDatum) => {
                        return [format(e.tm, 'MM/dd/yyyy'), e.data[s]]
                    }),
                    name: s,
                    type: 'line',
                    clip: false,
                    smooth: true,
                    sampling: 'average',
                    showSymbol: false,
                    width: 4,
                    xAxisIndex: 0,
                    yAxisIndex: 0,
                }
            }),
        }

        this.graph.setOption(options)
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