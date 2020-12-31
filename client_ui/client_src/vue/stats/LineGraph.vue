<template>
    <div ref="graphDiv" style="width: 100%; height: 100%;"></div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { StatXYSeriesData, XLineMarker, XAreaMarker } from '@client/js/stats/seriesData'
import { generateArrayRange } from '@client/js/array'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/line'
import 'echarts/lib/component/axisPointer'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/legend'
import 'echarts/lib/component/legendScroll'
import 'echarts/lib/component/tooltip'
import 'echarts/lib/component/markLine'
import 'echarts/lib/component/markArea'

@Component
export default class LineGraph extends Vue {
    graph : any | null = null

    $refs!: {
        graphDiv: HTMLElement
    }

    @Prop({type:Array, required: true})
    seriesData!: (StatXYSeriesData | null)[]

    @Prop({type:Boolean, default: false})
    separateGraphs!: boolean
    resizeTimeout: number | null = null

    @Prop()
    forcedMinX!: any | undefined

    @Prop()
    forcedMaxX!: any | undefined

    zoomStart: number = 0
    zoomEnd: number = 100

    get validSeriesData() : StatXYSeriesData[] {
        return <StatXYSeriesData[]>this.seriesData.filter((ele : StatXYSeriesData | null) => !!ele)
    }

    @Watch('separateGraphs')
    onChangeSeparateGraphs() {
        if (!!this.graph) {
            this.graph.clear()
        }
        this.refreshGraph()
    }

    @Watch('validSeriesData')
    refreshGraph() {
        // This needs to be up here so that the height computations work properly.
        if (!!this.graph) {
            this.graph.resize()
        }
        this.resizeTimeout = null

        if (this.validSeriesData.length == 0) { 
            this.graph.clear()   
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
            if (seriesToAxis.has(type) && !this.separateGraphs) {
                continue
            }

            let x: any = {
                type: type,
                nameTextStyle: {
                    color: '#FFFFFF'
                },
                axisLine: {
                    lineStyle: {
                        color: '#FFFFFF'
                    }
                },
                axisLabel: {
                    formatter: (value: any) => {
                        if (!this.validSeriesData[i]) {
                            return value
                        }
                        return this.validSeriesData[i].xFormatter(value)
                    }
                },
                inverse: this.validSeriesData[i].reversed,
                gridIndex: this.separateGraphs ? i : 0,
                position: 'top',
            }

            if (this.forcedMinX !== undefined) {
                x['min'] = this.forcedMinX
            }

            if (this.forcedMaxX !== undefined) {
                x['max'] = this.forcedMaxX
            }

            xAxis.push(x)
            seriesToAxis.set(type, xAxis.length - 1)
        }

        let yAxis: any[] = []
        for (let i = 0; i < ( this.separateGraphs ? this.validSeriesData.length : 1); ++i) {
            let axis: any = {
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
                gridIndex: i,
            }

            if (this.separateGraphs) {
                axis['name'] = this.validSeriesData[i]._name
                axis['nameLocation'] = 'center'
                axis['nameRotate'] = 90
                axis['nameGap'] = 50
            }

            yAxis.push(axis)
        }
    
        let grids: any[] = []

        const parentHeight = !!this.$refs.graphDiv ? this.$refs.graphDiv.offsetHeight : 650
        const gridHeightMarginPx = 40
        const gridTopReservedPx = 100
        const gridBottomReservedPx = 50
        const gridUsableHeightPx = parentHeight - gridTopReservedPx - gridBottomReservedPx - Math.max(this.validSeriesData.length - 1, 0) * gridHeightMarginPx 
        const gridHeight = this.separateGraphs ? gridUsableHeightPx / this.validSeriesData.length : gridUsableHeightPx

        for (let i = 0; i < ( this.separateGraphs ? this.validSeriesData.length : 1); ++i) {
            grids.push({
                show: false,
                left: 100,
                right: 100,
                top: gridTopReservedPx + (gridHeight + gridHeightMarginPx) * i,
                height: gridHeight,
            })
        }

        let dataZooms: any[] = [
            {
                type: 'slider',
                show: true,
                top: 30,
                start: this.zoomStart,
                end: this.zoomEnd,
                xAxisIndex: generateArrayRange(0, xAxis.length),
                textStyle: {
                    color: '#FFFFFF'
                }
            },
        ]

        let options : any = {
            grid: grids,
            legend: {
                show: true,
                type: 'scroll',
                textStyle: {
                    color: '#FFFFFF',
                },
            },
            xAxis: xAxis,
            yAxis: yAxis,
            axisPointer: {
                link: {xAxisIndex: 'all'}
            },
            dataZoom: dataZooms,
            tooltip: {
                trigger: 'axis',
            }
        }

        options.series = this.validSeriesData.map((series : StatXYSeriesData, seriesIdx: number) => ({
            data: series._x.map((x : number, idx : number) => {
                return [x, series._y[idx]]
            }),
            name: series._name,
            type: 'line',
            smooth: true,
            smoothMonotone: 'x',
            sampling: 'average',
            showSymbol: false,
            width: 4,
            xAxisIndex: this.separateGraphs ? seriesIdx : seriesToAxis.get(series._type)!,
            yAxisIndex: this.separateGraphs ? seriesIdx : 0,
            markLine: {
                label: {
                    show: true,
                    position: 'insideEndTop',
                    fontSize: 16,
                    formatter: (p: any) => {
                        return p.name
                    }
                },
                lineStyle: {
                    type: 'solid',
                    width: 3,
                },
                symbol: ['none', 'none'],
                data: series._xLines.map((ele: XLineMarker) => {
                    return  [
                        {
                            xAxis: ele.x,
                            yAxis: 'min',
                            name: ele.name,
                            lineStyle: {
                                color: ele.colorOverride
                            }
                        },
                        {
                            xAxis: ele.x,
                            yAxis: 'max',
                            symbol: ele.symbol,
                            symbolSize: 24,
                        }
                    ]
                })
            },
            markArea: {
                label: {
                    show: true,
                    position: 'inside',
                    fontSize: 16,
                    formatter: (p: any) => {
                        return p.name
                    }
                },
                itemStyle: {
                    color: 'rgba(255, 204, 203, 0.3)'
                },
                data: series._xAreas.map((ele: XAreaMarker) => {
                    return [
                        {
                            xAxis: ele.start,
                            yAxis: 'min',
                            name: ele.name,
                        },
                        {
                            xAxis: ele.end,
                            yAxis: 'max',
                        }
                    ]
                })
            }
        }))

        this.graph.setOption(options)
    }

    graphResize() {
        // The code is setup this way to request a resize instead of
        // actually performing the resize since we compute the height
        // of our grids manually - therefore we need to actually redraw
        // the whole graph to get things to resize dynamically.
        if (this.resizeTimeout !== null) {
            window.clearTimeout(this.resizeTimeout)
        }

        this.resizeTimeout = window.setTimeout(() => {
            this.refreshGraph()
        }, 100)
    }

    mounted() {
        this.graph = echarts.init(this.$refs.graphDiv, null, { renderer: 'canvas' })

        let zr = this.graph.getZr()
        zr.on('click', (params: any) => {
            let pointInPixel = [params.offsetX, params.offsetY];
            let numGrids = this.separateGraphs ? this.seriesData.length : 1

            for (let i = 0; i < numGrids; ++i) {
                const finder = {gridIndex: i}
                if (this.graph.containPixel(finder, pointInPixel)) {
                    let pointInGrid = this.graph.convertFromPixel(finder, pointInPixel);
                    this.$emit('graphclick', {
                        gridIndex: i,
                        pts: pointInGrid
                    })
                }
            }
        })

        this.graph.on('datazoom', (params: any) => {
            this.zoomStart = params.start
            this.zoomEnd = params.end
        })

        Vue.nextTick(() => {
            this.refreshGraph()
        })
        window.addEventListener('resize', this.graphResize)
    }
}

</script>

<style scoped>

</style>