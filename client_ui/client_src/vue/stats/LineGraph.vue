<template>
    <div ref="graphDiv" style="width: 100%; height: 100%;"></div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { StatXYSeriesData } from '@client/js/stats/seriesData'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/line'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/legend'
import 'echarts/lib/component/legendScroll'
import 'echarts/lib/component/tooltip'

@Component
export default class LineGraph extends Vue {
    graph : any | null = null

    $refs!: {
        graphDiv: HTMLElement
    }

    @Prop({type:Array, required: true})
    seriesData!: (StatXYSeriesData | null)[]

    get validSeriesData() : StatXYSeriesData[] {
        return <StatXYSeriesData[]>this.seriesData.filter((ele : StatXYSeriesData | null) => !!ele)
    }

    @Watch('validSeriesData')
    refreshGraph() {
        if (!!this.graph) {
            this.graph.clear()
        }

        if (this.validSeriesData.length == 0) {
            return
        }

        // Create a new x-axis for each new type of X-axis as determined by the series's type.
        // The series type determines what the "type" of the X-axis is, we assume that the Y axis is always
        // a numerical value for now.
        // TODO: Allow users to prevent the merging of axis?
        let seriesToAxis: Map<string, number> = new Map<string, number>()
        let xAxis : any[] = []
        for (let i = 0; i < this.validSeriesData.length; ++i) {
            let type = this.validSeriesData[i]._type
            if (seriesToAxis.has(type)) {
                continue
            }

            xAxis.push({
                type: type,
                nameTextStyle: {
                    color: '#FFFFFF'
                },
                axisLine: {
                    lineStyle: {
                        color: '#FFFFFF'
                    }
                },
                inverse: this.validSeriesData[i].reversed
            })
            seriesToAxis.set(type, xAxis.length - 1)
        }

        let options : any = {
            grid: {
                show: false,
            },
            legend: {
                show: true,
                type: 'scroll',
                textStyle: {
                    color: '#FFFFFF',
                },
            },
            xAxis: xAxis,
            yAxis: {
                type: 'value',
                nameTextStyle: {
                    color: '#FFFFFF'
                },
                axisLine: {
                    lineStyle: {
                        color: '#FFFFFF'
                    }
                },
                min: (value: any) => {
                    return Math.floor(value.min - (value.max - value.min) * 0.01)
                },
                max: (value: any) => {
                    return Math.ceil(value.max + (value.max - value.min) * 0.01)
                },
            },
            dataZoom: [
                {
                    type: 'slider',
                    show: true,
                    start: 0,
                    end: 100,
                    xAxisIndex: 0,
                },
                {
                    type: 'inside',
                    start: 0,
                    end: 100,
                    xAxisIndex: 0,
                },
            ],
            tooltip: {
                trigger: 'axis',
            }        
        }

        options.series = this.validSeriesData.map((series : StatXYSeriesData) => ({
            data: series._x.map((x : number, idx : number) => {
                return [x, series._y[idx]]
            }),
            name: series._name,
            type: 'line',
            smooth: false,
            width: 4,
            xAxisIndex: seriesToAxis.get(series._type)!,
        }))

        this.graph.setOption(options)
        this.graphResize()
    }

    graphResize() {
        if (!!this.graph) {
            this.graph.resize()
        }
    }

    mounted() {
        this.graph = echarts.init(this.$refs.graphDiv, null, { renderer: 'canvas' })
        this.refreshGraph()
        window.addEventListener('resize', this.graphResize)
        Vue.nextTick(() => {
            this.graphResize()
        })
    }
}

</script>

<style scoped>

</style>