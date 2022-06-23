<template>
    <div ref="graphDiv" style="width: 100%; height: 100%;"></div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { StatXYSeriesData } from '@client/js/stats/seriesData'
import { XLineMarker, XAreaMarker } from '@client/js/stats/graphData'
import { generateArrayRange } from '@client/js/array'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/line'
import 'echarts/lib/chart/custom'
import 'echarts/lib/component/axisPointer'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/grid'
import 'echarts/lib/component/legend'
import 'echarts/lib/component/legendScroll'
import 'echarts/lib/component/tooltip'
import 'echarts/lib/component/markLine'
import 'echarts/lib/component/markArea'
import 'echarts/lib/component/visualMap'

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

    @Prop({type: Boolean, default: false})
    diffGraph!: boolean

    @Prop({type: String, default: 'seconds'})
    unit!: string

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
        this.refreshGraph(this.validSeriesData, this.validSeriesData)
    }

    @Watch('validSeriesData')
    refreshGraph(newSeriesData: StatXYSeriesData[], oldSeriesData: StatXYSeriesData[]) {
        // This needs to be up here so that the height computations work properly.
        // Needs to in a nextTick because otherwise we might get an error saying that we
        // can't call resize in the main process?
        Vue.nextTick(() => {
            if (!!this.graph) {
                this.graph.resize()
            }
        })
        this.resizeTimeout = null

        // Looking for conditions where we want to clear the graph and redraw as we don't want to do this
        // *every* time we get some update to the series data (i.e. when we're only updating a marker).
        // The safe scenarios where we can clear as when there's no more data to display (validSeriesData.length)
        // or when some sort of incompatbile change happens in the series data. For example, if we add more data
        // or remove data we should trigger a redraw. Furthermore, if the axis types are different then it'll
        // be good to do a clear as well.
        if (this.validSeriesData.length == 0 || 
            newSeriesData.length != oldSeriesData.length || 
            newSeriesData.some((a: StatXYSeriesData, idx: number) => !a.compatibleWith(oldSeriesData[idx]))) { 
            this.graph.clear()   
            if (this.validSeriesData.length == 0) {
                return
            }
        }

        let groupSet: Set<string> = new Set()
        let groupColor: Map<string, string> = new Map()
        let groupIcon: Map<string, string> = new Map()
        if (this.separateGraphs) {
            // If we separate graphs then we create a new 'group' for each specified grouping.
            this.validSeriesData.forEach((g: StatXYSeriesData) => {
                if (g._allGroups) {
                    return
                }
                groupSet.add(g._group)

                if (!!g._groupIcon) {
                    groupIcon.set(g._group, g._groupIcon)
                }

                if (!!g._groupColor) {
                    groupColor.set(g._group, g._groupColor)
                }
            })
        } else {
            groupSet.add('Default')
        }

        let groupArr = Array.from(groupSet)
        let groupToGrid: Map<string, number> = new Map()
        groupArr.forEach((g: string, idx: number) => {
            groupToGrid.set(g, idx)
        })

        // Create a new x-axis for each new type of X-axis as determined by the series's type.
        // The series type determines what the "type" of the X-axis is, we assume that the Y axis is always
        // a numerical value for now.
        // TODO: Allow users to prevent the merging of axis?
        let groupSeriesAxis: Map<string, Map<string, number>> = new Map()
        let xAxis : any[] = []
        for (let i = 0; i < this.validSeriesData.length; ++i) {
            if (this.validSeriesData[i]._allGroups) {
                continue
            }
            let type = this.validSeriesData[i]._type
            let group = this.separateGraphs ? this.validSeriesData[i]._group : 'Default'
            if (!groupSeriesAxis.has(group)) {
                groupSeriesAxis.set(group, new Map())
            }

            let series = groupSeriesAxis.get(group)!
            if (series.has(type)) {
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
                gridIndex: groupToGrid.get(group)!,
                position: 'top',
            }

            if (this.forcedMinX !== undefined) {
                x['min'] = this.forcedMinX
            }

            if (this.forcedMaxX !== undefined) {
                x['max'] = this.forcedMaxX
            }

            xAxis.push(x)
            series.set(type, xAxis.length - 1)
        }

        let yAxis: any[] = []
        for (let i = 0; i < groupArr.length; ++i) {
            let axis: any = {
                type: 'value',
                nameTextStyle: {
                    rich: {
                        v: {},
                        icon: {},
                    }
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
                axis['name'] = `{icon|}{spacer|}{v|${groupArr[i]}}`
                axis['nameLocation'] = 'center'
                axis['nameRotate'] = 90
                axis['nameGap'] = 50
            }

            if (groupColor.has(groupArr[i])) {
                axis.nameTextStyle.rich.v.color = groupColor.get(groupArr[i])!
            } else {
                axis.nameTextStyle.rich.v.color = '#FFFFFF'
            }

            if (groupIcon.has(groupArr[i])) {
                axis.nameTextStyle.rich.icon = {
                    backgroundColor: {
                        image: groupIcon.get(groupArr[i])!,
                    },
                    width: 24,
                    height: 24,
                }

                axis.nameTextStyle.rich.spacer = {
                    width: 4,
                    height: 24,
                }
            }

            yAxis.push(axis)
        }
    
        let grids: any[] = []

        const parentHeight = !!this.$refs.graphDiv ? this.$refs.graphDiv.offsetHeight : 650
        const gridHeightMarginPx = 40
        const gridTopReservedPx = 100
        const gridBottomReservedPx = 20
        const gridUsableHeightPx = parentHeight - gridTopReservedPx - gridBottomReservedPx - (this.separateGraphs ? Math.max(groupArr.length - 1, 0) * gridHeightMarginPx : 0)
        const gridHeight = this.separateGraphs ? gridUsableHeightPx / groupArr.length : gridUsableHeightPx

        for (let i = 0; i < groupArr.length; ++i) {
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
                trigger: 'item',
            }
        }

        if (this.diffGraph) {
            // Need to compute min and max manually because if they're not set then ECharts will error out.
            // On the other hand, if they're just set to arbitrarily large values then the visual map won't apply.
            let minVal = (this.validSeriesData.length > 0) ? Math.min(...this.validSeriesData[0]._y) : 0
            let maxVal = (this.validSeriesData.length > 0) ? Math.max(...this.validSeriesData[0]._y) : 0

            options.visualMap = [{
                type: 'piecewise',
                show: false,
                pieces: [
                    {
                        lte: 0,
                        gt: minVal - 1,
                        color: '#FF5252',
                    },
                    {
                        gt: 0,
                        lte: maxVal + 1,
                        color: '#4CAF50'
                    }
                ] 
            }]
        }

        options.series = []
        let delayedSeries = []

        for (let series of this.validSeriesData) {
            let allGroups = !this.separateGraphs ? ['Default'] : series._allGroups ? groupArr : [series._group]
            for (let group of allGroups) {
                const grid = grids[groupToGrid.get(group)!]
                let opts: any = {
                    data: series._x.map((x : number, idx : number) => {
                        return [x, series._y[idx]]
                    }),
                    name: series._name,
                    type: 'line',
                    clip: true,
                    smooth: true,
                    sampling: 'average',
                    showSymbol: series.showSymbol,
                    symbol: series.showSymbol ? series._symbol : 'line',
                    symbolSize: 6,
                    width: 4,
                    xAxisIndex: groupSeriesAxis.get(group)!.get(series._type)!,
                    yAxisIndex: groupToGrid.get(group)!,
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
                                    y: grid.top + grid.height,
                                    name: ele.name,
                                    lineStyle: {
                                        color: ele.colorOverride
                                    }
                                },
                                {
                                    xAxis: ele.x,
                                    y: grid.top,
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
                                    y: grid.top + grid.height,
                                    name: ele.name,
                                },
                                {
                                    xAxis: ele.end,
                                    y: grid.top,
                                }
                            ]
                        })
                    },
                    tooltip: {
                        formatter: (params: any) => {
                            return `${params.marker} ${params.seriesName}: ${params.value[1]}@${params.value[0]} ${this.unit}`
                        }
                    }
                }

                if (series.hasStyle) {
                    opts.lineStyle = series.echartsLineStyle
                    opts.itemStyle = series.echartsItemStyle
                }
                options.series.push(opts)

                for (let o of series._overlayPeriods) {
                    let s = o.constructEchartsSeries(24, grid.top, this.unit)
                    s.itemStyle = {
                        opacity: 0.7
                    }
                    s.zlevel = 1
                    s.xAxisIndex = groupSeriesAxis.get(group)!.get(series._type)!
                    s.yAxisIndex = groupToGrid.get(group)!
                    delayedSeries.push(s)
                }
            }
        }

        options.series.push(...delayedSeries)
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
            this.refreshGraph(this.validSeriesData, this.validSeriesData)
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
            this.refreshGraph(this.validSeriesData, this.validSeriesData)
        })
        window.addEventListener('resize', this.graphResize)
    }

    beforeDestroy() {
        window.removeEventListener('resize', this.graphResize)
    }
}

</script>

<style scoped>

</style>