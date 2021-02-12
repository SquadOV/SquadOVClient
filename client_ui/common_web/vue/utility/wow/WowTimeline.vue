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

        <loading-container :is-loading="isLoading">
            <template v-slot:default="{ loading }">
                <div class="full-width" :style="timelineStyle" v-if="!loading">
                    <line-graph
                        :series-data="activeSeriesData"
                        :separate-graphs="separateGraphs"
                        :forced-min-x="encounterMinX"
                        :forced-max-x="encounterMaxX"
                        @graphclick="handleClick"
                    >
                    </line-graph>
                </div>
            </template>
        </loading-container>
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
import {
    dateRangeIntersects,
    dateClamp
} from '@client/js/time'
import LineGraph from '@client/vue/stats/LineGraph.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

const STAT_SECOND_STEP = 5.0
const BASE_GRAPH_HEIGHT = 650

@Component({
    components: {
        LoadingContainer,
        LineGraph
    }
})
export default class WowTimeline extends Vue {
    @Prop({type:Number, required: true})
    userId!: number

    @Prop({required: true})
    matchUuid!: string

    // Absolute start time - this is what we use to generate our timestamp strings.
    @Prop({required: true})    
    startTime!: Date

    // Start and end time of the current encounter. This is the time period we actually display.
    @Prop({required: true})    
    encounterStartTime!: Date

    @Prop({required: true})    
    encounterEndTime!: Date

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
    pendingEndpoints: Set<string> = new Set()

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
        // Relative to startTime because we're the way the graph is setup, the time along
        // the X axis is still going to be relative to the absolute start time of the entire
        // match and what we're receiving in pts is just the (x, y) coordinates effectively.
        let dt = new Date(this.startTime.getTime() + evt.pts[0] * 1000.0)
        this.$emit('go-to-time', dt)
    }

    get expectedXRange(): number[] {
        let ret: number[] = []
        let range = (this.encounterEndTime.getTime() - this.encounterStartTime.getTime()) / 1000
        for (let x = 0; x <= range; x += STAT_SECOND_STEP) {
            ret.push(x)
        }
        return ret
    }

    get activeEndpoint(): string {
        return this.data[this.selectedTab].endpoint
    }

    get isLoading(): boolean {
        return !(this.activeEndpoint in this.cachedStats)
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

    get encounterMinX(): number {
        return this.convertTmToX(this.encounterStartTime)
    }

    get encounterMaxX(): number {
        return this.convertTmToX(this.encounterEndTime)
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
                paddedX.map((ele: number) => {
                    // Need to account for the offset between the current encounter
                    // and the start of the match.
                    return ele + (this.encounterStartTime.getTime() - this.startTime.getTime()) / 1000.0
                }),
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
                    if (!!e.death && e.death.guid == guid) {
                        data.addXMarkLine({
                            x: this.convertTmToX(e.tm),
                            name: `Death`,
                            symbol: `image://assets/wow/stats/skull.png`
                        })
                    } else if (!!e.resurrect && e.resurrect.guid == guid) {
                        data.addXMarkLine({
                            x: this.convertTmToX(e.tm),
                            name: `Resurrect`,
                            symbol: `image://assets/wow/stats/res.png`
                        })
                    }
                }
            }

            if (this.showBloodlust && !!this.events) {
                for (let aura of this.events.auras) {
                    if (!dateRangeIntersects(aura.appliedTm, aura.removedTm, this.encounterStartTime, this.encounterEndTime)) {
                        continue
                    }

                    if (wowc.BLOODLUST_SPELL_IDS.has(aura.spellId)) {
                        data.addXMarkArea({
                            start: this.convertTmToX(dateClamp(aura.appliedTm, this.encounterStartTime, this.encounterEndTime)),
                            end: this.convertTmToX(dateClamp(aura.removedTm, this.encounterStartTime, this.encounterEndTime)),
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

    @Watch('encounterStartTime')
    @Watch('encounterEndTime')
    forceRefreshData() {
        Vue.delete(this.cachedStats, this.activeEndpoint)
        this.refreshData()
    }

    @Watch('selectedTab')
    refreshData() {
        if (this.activeEndpoint in this.cachedStats) {
            return
        }

        if (this.pendingEndpoints.has(this.activeEndpoint)) {
            return
        }

        this.pendingEndpoints.add(this.activeEndpoint)

        let endpoint = this.activeEndpoint
        apiClient.getWoWMatchStats(this.userId, this.matchUuid, endpoint, {
            psStepSeconds: STAT_SECOND_STEP,
            start: this.encounterStartTime.getTime(),
            end: this.encounterEndTime.getTime()
        }).then((resp: ApiData<WowMatchStatContainer>) => {
            Vue.set(this.cachedStats, endpoint, resp.data)
        }).catch((err: any) => {
            console.log('Failed to get WoW match stats: ', err)
        }).finally(() => {
            this.pendingEndpoints.delete(endpoint)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>