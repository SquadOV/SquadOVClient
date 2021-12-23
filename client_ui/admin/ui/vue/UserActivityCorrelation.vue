<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <date-range-picker
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            >
            </date-range-picker>

            <v-select
                v-model="mode"
                :items="modeItems"
                label="X-Axis"
                hide-details
            >
            </v-select>

            <v-checkbox
                v-model="scatter"
                dense
                hide-details
                label="Scatter Plot"
            >
            </v-checkbox>
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
import { ActivityDatum, getActivityScatter } from '@client/ts/activity'
import * as echarts from 'echarts/lib/echarts.js'
import * as ecSimpleTransform from 'echarts-simple-transform'
import 'echarts/lib/chart/boxplot'
import 'echarts/lib/chart/scatter'
import 'echarts/lib/component/dataset'
import 'echarts/lib/component/grid'
import 'echarts/lib/component/tooltip'
import 'echarts/lib/component/dataZoom'
import 'echarts/lib/component/grid'
import 'echarts/lib/component/legend'
import 'echarts/lib/component/legendScroll'
import 'echarts/lib/component/tooltip'
import 'echarts/lib/component/transform'

echarts.registerTransform(ecSimpleTransform.aggregate)

@Component({
    components: {
        DateRangePicker,
    }
})
export default class UserActivityCorrelation extends Vue {
    startDate: Date = new Date()
    endDate: Date = new Date()
    mode: number=  0
    scatter: boolean = true

    data: ActivityDatum[] = []
    graph : any | null = null

    $refs!: {
        graphDiv: HTMLElement
    }

    get modeItems(): any[] {
        return [
            {
                text: 'Account Age (Days)',
                value: 0,
            },
            {
                text: 'Days of VOD Recordings',
                value: 1,
            },
            {
                text: '# of Squads',
                value: 2,
            },
            {
                text: '# of Squads (non-public)',
                value: 3,
            },
            {
                text: '# of Squadmates',
                value: 4,
            },
            {
                text: '# of Squadmates (non-public)',
                value: 5,
            },
            {
                text: '# of VODs (in time period)',
                value: 6,
            },
            {
                text: '# of Clips (in time period)',
                value: 7,
            },
            {
                text: '# of Share Links (excluding clips)',
                value: 8,
            },
            {
                text: '# of Share Links (including clips)',
                value: 9,
            },
            {
                text: 'Most Played Game',
                value: 10,
            },
            {
                text: 'Internet Speed (rounded)',
                value: 11,
            },
            {
                text: 'CPU',
                value: 12,
            },
            {
                text: 'GPU',
                value: 13,
            },
            {
                text: 'Email Verified',
                value: 14,
            },
            {
                text: '# of Squads (public)',
                value: 15,
            },
            {
                text: '# hours of VODs (rounded)',
                value: 16,
            },
        ]
    }

    @Watch('startDate')
    @Watch('endDate')
    @Watch('mode')
    refreshData() {
        this.data = []

        getActivityScatter(this.startDate, this.endDate, this.mode).then((resp: ActivityDatum[]) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get activity scatter: ' , err)
        })
    }

    @Watch('data')
    @Watch('scatter')
    refreshGraph() {
        if (!this.graph || this.data.length == 0) {
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

        let options : any = {
            legend: {
                show: true,
                type: 'scroll',
                textStyle: {
                    color: '#FFFFFF',
                },
            },
            xAxis: {
                type: (this.mode === 10 || this.mode === 12 || this.mode === 13 || this.mode === 14 || !this.scatter) ? 'category' : 'value',
                name: this.modeItems[this.mode].text,
            },
            yAxis: {
                type: 'value',
                name: 'Days Active in Period',
            },
            dataZoom: dataZooms,
            tooltip: this.scatter ? {
            } : {
                trigger: 'item'
            },
            dataset: this.scatter ? [
            ] : [
                {
                    source: [
                        ['x', 'y'],
                        ...this.data.map((ele: ActivityDatum) => {
                            return [ele.x, ele.y]
                        })
                    ]
                },
                {
                    transform: [
                        {
                            type: 'ecSimpleTransform:aggregate',
                            config: {
                                resultDimensions: [
                                    { name: 'min', from: 'y', method: 'min' },
                                    { name: 'Q1', from: 'y', method: 'Q1' },
                                    { name: 'median', from: 'y', method: 'median' },
                                    { name: 'Q3', from: 'y', method: 'Q3' },
                                    { name: 'max', from: 'y', method: 'max' },
                                    { name: 'x', from: 'x' }
                                ],
                                groupBy: 'x',
                            }
                        },
                        {
                            type: 'sort',
                            config: {
                                dimension: 'x',
                                order: 'asc'
                            }
                        }
                    ]
                },
            ],
            series: this.scatter ? {
                type: 'scatter',
                encode: { tooltip: [0, 1] },
                data: this.data.map((ele: ActivityDatum) => {
                    return [ele.x, ele.y]
                }),
                blendMode: 'source-over',
                large: true,
                largeThreshold: 800,
            } : {
                type: 'boxplot',
                datasetIndex: 1,
                encode: {
                    x: 'x',
                    y: ['min', 'Q1', 'median', 'Q3', 'max'],
                    itemName: ['x'],
                    tooltip: ['min', 'Q1', 'median', 'Q3', 'max']
                }
            },
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