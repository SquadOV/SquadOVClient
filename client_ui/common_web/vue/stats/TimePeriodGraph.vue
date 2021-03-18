<template>
    <div ref="graphDiv" style="width: 100%; height: 100%;"></div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { StatTimePeriodData } from '@client/js/stats/periodData'
import { XLineMarker, XAreaMarker } from '@client/js/stats/graphData'
import { generateArrayRange } from '@client/js/array'
import * as echarts from 'echarts/lib/echarts.js'
import 'echarts/lib/chart/custom'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/tooltip'

@Component
export default class TimePeriodGraph extends Vue {
    graph : any | null = null

    $refs!: {
        graphDiv: HTMLElement
    }

    @Prop({type:Array, required: true})
    data!: (StatTimePeriodData | null)[]

    @Prop()
    forcedMinX!: number

    @Prop()
    forcedMaxX!: number

    @Prop({default: 60})
    maxSpan!: number

    @Prop({ default: 48 })
    layerHeight!: number

    resizeTimeout: number | null = null
    zoomStart: number = 0
    zoomEnd: number = 0

    get validData() : StatTimePeriodData[] {
        return <StatTimePeriodData[]>this.data.filter((ele : StatTimePeriodData | null) => !!ele)
    }

    @Watch('validData')
    refreshGraph(newData: StatTimePeriodData[], oldData: StatTimePeriodData[]) {
        Vue.nextTick(() => {
            if (!!this.graph) {
                this.graph.resize()
            }
        })
        this.resizeTimeout = null
        
        if (this.validData.length == 0 || 
            newData.length != oldData.length || 
            newData.some((a: StatTimePeriodData, idx: number) => !a.compatibleWith(oldData[idx]))) { 
            this.graph.clear()   
            if (this.validData.length == 0) {
                return
            }
        }

        let groupSet: Set<string> = new Set()
        let groupColor: Map<string, string> = new Map()
        let groupIcon: Map<string, string> = new Map()
        this.validData.forEach((g: StatTimePeriodData) => {
            groupSet.add(g._group)

            if (!!g._groupIcon) {
                groupIcon.set(g._group, g._groupIcon)
            }

            if (!!g._groupColor) {
                groupColor.set(g._group, g._groupColor)
            }
        })

        let groupArr = Array.from(groupSet).sort((a: string, b: string) => {
            if (a < b) {
                return -1
            } else if (a > b) {
                return 1
            } else {
                return 0
            }
        })

        let dataZooms: any[] = [
            {
                type: 'slider',
                show: true,
                top: 30,
                startValue: this.zoomStart,
                endValue: this.zoomEnd,
                maxValueSpan: this.maxSpan,
                xAxisIndex: generateArrayRange(0, groupArr.length),
                textStyle: {
                    color: '#FFFFFF'
                }
            },
        ]

        let options : any = {
            xAxis: [],
            grid: [],
            yAxis: [],
            dataZoom: dataZooms,
            tooltip: {
                trigger: 'item',
            },
            series: [],
        }

        const parentHeight = !!this.$refs.graphDiv ? this.$refs.graphDiv.offsetHeight : 650
        const gridHeightMarginPx = 40
        const gridTopReservedPx = 100
        const gridBottomReservedPx = 20
        const gridUsableHeightPx = parentHeight - gridTopReservedPx - gridBottomReservedPx - Math.max(groupArr.length - 1, 0) * gridHeightMarginPx
        let currentGridTop: number = gridTopReservedPx

        this.validData.forEach((ele: StatTimePeriodData, idx: number) => {
            const layerHeight = ele.numDisplayLayers * this.layerHeight
            const gridHeight = Math.max(layerHeight, 150)
            let grid = {
                show: false,
                left: 50,
                right: 50,
                top: currentGridTop,
                height: gridHeight,
            }
            options.grid.push(grid)
            currentGridTop += (gridHeight + gridHeightMarginPx)

            let yAxis = {
                show: true,
                axisLine: {
                    lineStyle: {
                        color: '#FFFFFF'
                    }
                },
                axisTick: {
                    show: false,
                },
                minorTick: {
                    show: false,
                },
                axisLabel: {
                    show: false,
                },
                splitLine: {
                    show: false,
                },
                gridIndex: idx,
                name: `{icon|}{spacer|}{v|${ele._group}}`,
                nameLocation: 'center',
                nameRotate: 90,
                nameGap: 20,
                nameTextStyle: {
                    rich: {
                        v: {
                            color: groupColor.get(ele._group)!
                        },
                        icon: {
                            backgroundColor: {
                                image: groupIcon.get(ele._group)!,
                            },
                            width: 24,
                            height: 24,
                        },
                        spacer: {
                            width: 4,
                            height: 24
                        }
                    }
                }
            }
            options.yAxis.push(yAxis)

            let xAxis = {
                type: 'value',
                nameTextStyle: {
                    color: '#FFFFFF'
                },
                axisLine: {
                    lineStyle: {
                        color: '#FFFFFF'
                    }
                },
                min: this.forcedMinX,
                max: this.forcedMaxX,
                gridIndex: idx,
                axisLabel: {
                    formatter: (value: any) => {
                        return ele.xFormatter(value)
                    }
                },
                position: 'top',
            }
            options.xAxis.push(xAxis)

            let s = ele.constructEchartsSeries(this.layerHeight, grid.top + (gridHeight - layerHeight) / 2.0, 'seconds')
            s.xAxisIndex = idx
            s.yAxisIndex = idx
            s.markLine = {
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
                data: ele._xLines.map((ele: XLineMarker) => {
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
            }

            s.markArea = {
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
                data: ele._xAreas.map((ele: XAreaMarker) => {
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
            }

            options.series.push(s)
        })

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
            this.refreshGraph(this.validData, this.validData)
        }, 100)
    }

    mounted() {
        this.graph = echarts.init(this.$refs.graphDiv, null, { renderer: 'canvas' })
        this.zoomStart = 0
        this.zoomEnd = this.zoomStart + this.maxSpan

        let zr = this.graph.getZr()
        zr.on('click', (params: any) => {
            let pointInPixel = [params.offsetX, params.offsetY];

            const finder = {gridIndex: 0}
            if (this.graph.containPixel(finder, pointInPixel)) {
                let pointInGrid = this.graph.convertFromPixel(finder, pointInPixel);
                this.$emit('graphclick', {
                    gridIndex: 0,
                    pts: pointInGrid
                })
            }
        })

        this.graph.on('datazoom', (params: any) => {
            this.zoomStart = params.startValue
            this.zoomEnd = params.endValue
        })

        Vue.nextTick(() => {
            this.refreshGraph(this.validData, this.validData)
        })
        window.addEventListener('resize', this.graphResize)
    }

    beforeDestroy() {
        window.removeEventListener('resize', this.graphResize)
    }
}

</script>