<template>
    <div class="full-width">
        <div class="d-flex align-center">
            <v-tabs v-model="selectedTab">
                <v-tab
                    v-for="(d, idx) in data"
                    :key="`tab-${idx}`"
                >
                    {{ d.title }}
                </v-tab>
            </v-tabs>
            
            <v-dialog
                v-model="showSettings"
                max-width="60%"
            >
                <template v-slot:activator="{on}">
                    <v-btn
                        v-on="on"
                        icon
                    >
                        <v-icon>
                            mdi-cog
                        </v-icon>
                    </v-btn>
                </template>
                
                <v-card>
                    <v-card-title>
                        Graph Settings
                    </v-card-title>
                    <v-divider></v-divider>

                    <div class="d-flex flex-wrap justify-space-around">
                        <v-checkbox
                            class="mx-2"
                            v-model="separateGraphs"
                            label="Separate Graphs"
                            dense
                            hide-details
                        >
                        </v-checkbox>

                        <v-checkbox
                            class="mx-2"
                            v-model="showTime"
                            label="Show Time"
                            dense
                            hide-details
                        >
                        </v-checkbox>

                        <v-checkbox
                            class="mx-2"
                            v-model="showEvents"
                            label="Show Events"
                            dense
                            hide-details
                        >
                        </v-checkbox>

                        <v-checkbox
                            class="mx-2"
                            v-model="showBloodlust"
                            label="Show Bloodlust"
                            dense
                            hide-details
                        >
                        </v-checkbox>
                    </div>

                    <v-card-actions>
                        <v-spacer></v-spacer>
                        <v-btn
                            color="success"
                            @click="showSettings = false"
                        >
                            Close
                        </v-btn>
                    </v-card-actions>
                </v-card>
            </v-dialog>
        </div>

        <div class="full-width" :style="timelineStyle">
            <line-graph
                :series-data="activeSeriesData"
                :separate-graphs="separateGraphs"
                @graphclick="handleClick"
            >
            </line-graph>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import {
    UnifiedWowEventContainer,
    SerializedWowMatchEvents
} from '@client/js/wow/events'
import {
    WowStatQueryParam,
    WowStatDatum,
    WowMatchStatContainer
} from '@client/js/wow/stats'
import {
    WowCharacter
} from '@client/js/wow/character'
import * as wowc from '@client/js/wow/constants'
import * as colors from '@client/js/wow/colors'
import { colorToCssString } from '@client/js/color'
import { StatXYSeriesData } from '@client/js/stats/seriesData'
import { apiClient, ApiData } from '@client/js/api'
import LineGraph from '@client/vue/stats/LineGraph.vue'

const STAT_SECOND_STEP = 5.0
const STAT_SECOND_INTERVAL = 10.0
const BASE_GRAPH_HEIGHT = 650

@Component({
    components: {
        LineGraph
    }
})
export default class WowTimeline extends Vue {
    @Prop({type:Number, required: true})
    userId!: number

    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})    
    startTime!: Date

    @Prop({required: true})    
    endTime!: Date

    @Prop({required: true})    
    currentTime!: Date | null

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({required: true})
    events!: SerializedWowMatchEvents | null

    @Prop({required: true})
    unifiedEvents!: UnifiedWowEventContainer[]

    selectedTab: number = 0
    cachedStats: {[v:string]: WowMatchStatContainer} = {}

    // Graph settings
    showSettings: boolean = false
    separateGraphs: boolean = true
    showEvents: boolean = true
    showBloodlust: boolean = true
    showTime: boolean = true

    data: any[] = [
        {
            title: 'Damage Done',
            endpoint: 'dps'
        },
        {
            title: 'Heals',
            endpoint: 'hps'
        },
        {
            title: 'Damage Received',
            endpoint: 'drps'
        },
    ]

    handleClick(evt: {gridIndex: number, pts: number[]}) {
        let dt = new Date(this.startTime.getTime() + evt.pts[0] * 1000.0)
        this.$emit('go-to-time', dt)
    }

    get expectedXRange(): number[] {
        let ret: number[] = []
        let range = (this.endTime.getTime() - this.startTime.getTime()) / 1000
        for (let x = 0; x <= range; x += STAT_SECOND_STEP) {
            ret.push(x)
        }
        return ret
    }

    get activeEndpoint(): string {
        return this.data[this.selectedTab].endpoint
    }

    get activeStatContainer(): WowMatchStatContainer {
        if (!(this.activeEndpoint in this.cachedStats)) {
            return {}
        }
        return this.cachedStats[this.activeEndpoint]
    }

    convertTmToX(tm: Date) : number {
        return (tm.getTime() - this.startTime.getTime()) / 1000
    }

    get activeSeriesData(): StatXYSeriesData[] {
        let series: StatXYSeriesData[] = []

        for (let [guid, stats] of Object.entries(this.activeStatContainer)) {
            let name = this.guidToName.has(guid) ?
                this.guidToName.get(guid)! :
                guid

            let x = (<WowStatDatum[]>stats).map((ele: WowStatDatum) => ele.tm)
            let y = (<WowStatDatum[]>stats).map((ele: WowStatDatum) => ele.value)

            let paddedX: number[] = []
            let paddedY: number[] = []

            for (let ex of this.expectedXRange) {
                if (x.length > 0 && y.length > 0) {
                    if (Math.abs(ex - x[0]) < 1e-3) {
                        paddedX.push(x.shift()!)
                        paddedY.push(y.shift()!)
                    } else {
                        paddedX.push(ex)
                        paddedY.push(0.0)    
                    }
                } else {
                    paddedX.push(ex)
                    paddedY.push(0.0)
                }
            }

            let data = new StatXYSeriesData(
                paddedX,
                paddedY,
                'value',
                'elapsedSeconds',
                name
            )

            if (!!this.currentTime && this.showTime) {
                data.addXMarkLine({
                    x: this.convertTmToX(this.currentTime),
                    name: '',
                    symbol: 'none',
                    colorOverride: colorToCssString(colors.getSelfColor()),
                })
            }

            if (this.showEvents) {
                for (let e of this.unifiedEvents) {
                    if (!!e.death) {
                        if (e.death.guid == guid) {
                            data.addXMarkLine({
                                x: this.convertTmToX(e.tm),
                                name: `Death`,
                                symbol: `image://assets/wow/stats/skull.png`
                            })
                        }
                    }
                }
            }

            if (this.showBloodlust && !!this.events) {
                for (let aura of this.events.auras) {
                    if (wowc.BLOODLUST_SPELL_IDS.has(aura.spellId)) {
                        data.addXMarkArea({
                            start: this.convertTmToX(aura.appliedTm),
                            end: this.convertTmToX(aura.removedTm),
                            name: aura.spellName
                        })
                    }
                }
            }

            series.push(data)
        }

        series.sort((a: StatXYSeriesData, b: StatXYSeriesData) => {
            if (a._name < b._name) {
                return -1
            } else if (a._name > b._name) {
                return 1
            } else {
                return 0
            }
        })

        return series
    }

    get timelineStyle(): any {
        let height = this.separateGraphs ? 
            Math.max(250 * this.activeSeriesData.length, BASE_GRAPH_HEIGHT) :
            BASE_GRAPH_HEIGHT

        return {
            'height': `${height}px`
        }
    }

    get guidToName(): Map<string, string> {
        let mapping = new Map<string, string>()
        for (let m of this.matchCharacters) {
            mapping.set(m.guid, m.name)
        }
        return mapping
    }

    @Watch('selectedTab')
    refreshData() {
        if (this.activeEndpoint in this.cachedStats) {
            return
        }

        let endpoint = this.activeEndpoint
        apiClient.getWoWMatchStats(this.userId, this.matchUuid, endpoint, {
            psStepSeconds: STAT_SECOND_STEP,
            psIntervalSeconds: STAT_SECOND_INTERVAL,
            start: this.startTime.getTime(),
            end: this.endTime.getTime()
        }).then((resp: ApiData<WowMatchStatContainer>) => {
            Vue.set(this.cachedStats, endpoint, resp.data)
        }).catch((err: any) => {
            console.log('Failed to get WoW match stats: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>