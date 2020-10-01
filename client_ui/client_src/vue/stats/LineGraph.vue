<template>
    <div ref="graphDiv" style="width: 100%; height: 600px;"></div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { StatXYSeriesData } from '@client/js/stats/series_data'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/line'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/legend'
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
        if (this.validSeriesData.length == 0) {
            return
        }

        let options : any = {
            xAxis: {
                type: 'category',
                data: this.validSeriesData[0]._x,
                nameTextStyle: {
                    color: '#FFFFFF'
                },
                axisLine: {
                    lineStyle: {
                        color: '#FFFFFF'
                    }
                }
            },
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
            data: series._y,
            type: 'line',
            smooth: true,
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
        this.graphResize()
        window.addEventListener('resize', this.graphResize)
    }
}

</script>

<style scoped>

</style>