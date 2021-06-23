<template>
    <div class="full-width">
        <div class="d-flex align-center pa-2">
            <div class="text-h6 font-weight-bold">
                Display Options: 
            </div>

            <wow-character-chooser
                :characters="matchCharacters"
                v-model="charactersToDisplay"
                :patch="patch"
                multiple
                style="max-width: 500px;"
            >
            </wow-character-chooser>

            <v-dialog
                v-model="showSettings"
                max-width="60%"
            >
                <template v-slot:activator="{on}">
                    <v-btn
                        class="my-1"
                        v-on="on"
                        color="primary"
                    >
                        Settings
                    </v-btn>
                </template>
                
                <v-card>
                    <v-card-title>
                        Graph Settings
                    </v-card-title>
                    <v-divider></v-divider>

                    <div class="pa-4">
                        <div class="text-overline">
                            Grouping
                        </div>
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
                                v-model="aggregateStats"
                                label="Aggregate"
                                dense
                                hide-details
                                :disabled="separateGraphs"
                            >
                            </v-checkbox>
                        </div>

                        <div class="text-overline">
                            Stats
                        </div>
                        <v-divider></v-divider>
                        <div class="d-flex flex-wrap justify-space-around">
                            <v-checkbox
                                class="mx-2"
                                v-model="showDps"
                                label="Damage Done"
                                dense
                                hide-details
                            >
                            </v-checkbox>

                            <v-checkbox
                                class="mx-2"
                                v-model="showHps"
                                label="Heals"
                                dense
                                hide-details
                            >
                            </v-checkbox>

                            <v-checkbox
                                class="mx-2"
                                v-model="showDrps"
                                label="Damage Received"
                                dense
                                hide-details
                            >
                            </v-checkbox>
                        </div>

                        <div class="text-overline">
                            Markers
                        </div>
                        <v-divider></v-divider>
                        <div class="d-flex flex-wrap justify-space-around">
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

                            <v-checkbox
                                class="mx-2"
                                v-model="showAuras"
                                label="Show Important Auras"
                                dense
                                hide-details
                                :disabled="!separateGraphs"
                            >
                            </v-checkbox>

                            <v-checkbox
                                class="mx-2"
                                v-model="showSpells"
                                label="Show Important Spells"
                                dense
                                hide-details
                                :disabled="!separateGraphs"
                            >
                            </v-checkbox>
                        </div>
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
                        ref="graph"
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
import Component, { mixins } from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import {
    UnifiedWowEventContainer,
    SerializedWowMatchEvents,
    isWowAuraBuff
} from '@client/js/wow/events'
import {
    WowStatQueryParam,
    WowStatDatum,
    WowMatchStatContainer
} from '@client/js/wow/stats'
import {
    WowCharacter,
    specIdToClassId,
} from '@client/js/wow/character'
import * as wowc from '@client/js/wow/constants'
import * as colors from '@client/js/wow/colors'
import { colorToCssString, colorFromElementTheme } from '@client/js/color'
import { StatXYSeriesData, LineStyle } from '@client/js/stats/seriesData'
import { apiClient, ApiData } from '@client/js/api'
import {
    dateRangeIntersects,
    dateClamp
} from '@client/js/time'
import { wowCache } from '@client/js/wow/staticCache'
import { staticClient } from '@client/js/staticData'
import { StatTimePeriodData, StatTimePeriodTrackData, StatTimePeriodSection } from '@client/js/stats/periodData'
import LineGraph from '@client/vue/stats/LineGraph.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import WowCharacterChooser from '@client/vue/utility/wow/WowCharacterChooser.vue'
import CommonComponent from '@client/vue/CommonComponent'

const STAT_SECOND_STEP = 5.0
const BASE_GRAPH_HEIGHT = 650

const DPS_ENDPOINT = 'dps'
const DRPS_ENDPOINT = 'drps'
const HPS_ENDPOINT = 'hps'

function endpointToStatName(endpoint: string): string {
    if (endpoint == DPS_ENDPOINT) {
        return 'Damage Dealt'
    } else if (endpoint == DRPS_ENDPOINT) {
        return 'Damage Received'
    } else if (endpoint == HPS_ENDPOINT) {
        return 'Heals'
    } else {
        return 'Unknown'
    }
}

@Component({
    components: {
        LoadingContainer,
        LineGraph,
        WowCharacterChooser,
    }
})
export default class WowTimeline extends mixins(CommonComponent) {
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

    @Prop({default: 0})
    friendlyTeam!: number

    @Prop({required: true})
    patch!: string

    @Prop({default: ''})
    charGuid!: string | undefined

    $refs!: {
        graph: LineGraph
    }

    cachedStats: {[v:string]: WowMatchStatContainer} = {}
    pendingEndpoints: Set<string> = new Set()
    spellIdNames: {[id: number]: string} = {}
    charactersToDisplay: WowCharacter[] = []

    // Graph settings
    showSettings: boolean = false
    
    separateGraphs: boolean = true
    aggregateStats: boolean = false

    showDps: boolean = true
    showHps: boolean = true
    showDrps: boolean = true

    showEvents: boolean = true
    showBloodlust: boolean = true
    showTime: boolean = true
    showAuras: boolean = false
    showSpells: boolean = false

    get displayGuidSet(): Set<string> {
        return new Set(this.charactersToDisplay.map((ele: WowCharacter) => ele.guid))
    }

    get friendlyCharacters(): WowCharacter[] {
        if (!this.matchCharacters) {
            return []
        }
        return this.matchCharacters.filter((c: WowCharacter) => {
            return c.team == this.friendlyTeam
        })
    }

    get enemyCharacters(): WowCharacter[] {
        if (!this.matchCharacters) {
            return []
        }
        return this.matchCharacters.filter((c: WowCharacter) => {
            return c.team != this.friendlyTeam
        })
    }

    handleClick(evt: {gridIndex: number, pts: number[]}) {
        // Relative to startTime because we're the way the graph is setup, the time along
        // the X axis is still going to be relative to the absolute start time of the entire
        // match and what we're receiving in pts is just the (x, y) coordinates effectively.
        let dt = new Date(this.startTime.getTime() + evt.pts[0] * 1000.0)
        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.GoToTimeline,
            'Timeline',
            dt.getTime()
        )
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

    get isLoading(): boolean {
        return this.pendingEndpoints.size > 0
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

    get endpoints(): string[] {
        let endpoints: string[] = []
        if (this.showDps) {
            endpoints.push(DPS_ENDPOINT)
        }

        if (this.showDrps) {
            endpoints.push(DRPS_ENDPOINT)
        }

        if (this.showHps) {
            endpoints.push(HPS_ENDPOINT)
        }
        return endpoints
    }

    get uniqueGroups(): string[] {
        let groups: Set<string> = new Set()
        for (let e of this.endpoints) {
            if (!(e in this.cachedStats)) {
                continue
            }

            for (let [guid, _stats] of Object.entries(this.cachedStats[e])) {
                if (!this.displayGuidSet.has(guid)) {
                    continue
                }
                let name = this.guidToName.has(guid) ?
                    this.guidToName.get(guid)! :
                    guid
                groups.add(name)
            }
        }

        return Array.from(groups)
    }

    get activeSeriesData(): StatXYSeriesData[] {
        let series: StatXYSeriesData[] = []

        let markerEndpoint = this.showDps ? DPS_ENDPOINT :
            this.showDrps ? DRPS_ENDPOINT :
            this.showHps ? HPS_ENDPOINT : ''

        const validSymbols = ['circle', 'rect', 'triangle', 'diamond', 'pin', 'arrow']
        let guidToSymbol: Map<string, string> = new Map()
        let guidToLineStyle: Map<string, LineStyle> = new Map()
        let specIdCount: Map<number, number> = new Map()

        for (let c of this.matchCharacters) {
            let classId = specIdToClassId(c.specId)
            let dupSpecCount = specIdCount.has(classId) ? specIdCount.get(classId)! : 0
            let style = {
                color: colorToCssString(colorFromElementTheme(this.$parent.$el, colors.specIdToColor(c.specId))),
            }
            guidToSymbol.set(c.guid, validSymbols[dupSpecCount % validSymbols.length])
            specIdCount.set(classId, dupSpecCount + 1)
            guidToLineStyle.set(c.guid, style)
        }

        let endpointToLineStyle: Map<string, LineStyle> = new Map()
        endpointToLineStyle.set(DPS_ENDPOINT, {
            color: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-wow-dps'))
        })
        endpointToLineStyle.set(HPS_ENDPOINT, {
            color: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-wow-hps'))
        })
        endpointToLineStyle.set(DRPS_ENDPOINT, {
            color: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-wow-drps'))
        })

        let endpointToSymbol: Map<string, string> = new Map()
        endpointToSymbol.set(DPS_ENDPOINT, 'circle')
        endpointToSymbol.set(HPS_ENDPOINT, 'triangle')
        endpointToSymbol.set(DRPS_ENDPOINT, 'diamond')

        // Make a request to cache all relevant spell and aura IDs.
        let spellIdsToCache: Set<number> = new Set()
        if (!!this.events) {
            for (let aura of this.events.auras) {
                spellIdsToCache.add(aura.spellId)
            }

            for (let sc of this.events.spellCasts) {
                spellIdsToCache.add(sc.spellId)
            }
        }

        wowCache.getCache(this.patch).bulkGetSpellNames(Array.from(spellIdsToCache)).then((resp: Map<number, string>) => {
            for (let [key, value] of resp) {
                Vue.set(this.spellIdNames, key, value)
            }
        })

        let addTimeToSeries = (data: StatXYSeriesData) => {
             if (this.showTime && !!this.currentTime) {
                data.addXMarkLine({
                    x: this.convertTmToX(this.currentTime),
                    name: '',
                    symbol: 'none',
                    colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-self')),
                })
            }
        }

        let addEventsToSeries = (data: StatXYSeriesData, filterGuid: string | undefined) => {
            if (this.showEvents) {
                for (let e of this.unifiedEvents) {
                    if (!!e.death && (!filterGuid || e.death.guid == filterGuid)) {
                        let guid = e.death.guid
                        let playerName = this.guidToName.has(guid) ?
                            this.guidToName.get(guid)! :
                            guid
                        data.addXMarkLine({
                            x: this.convertTmToX(e.tm),
                            name: `${playerName} - Death`,
                            symbol: `image://assets/wow/stats/skull.png`,
                            colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, colors.specIdToColor(this.guidToSpecId.get(guid)!))),
                        })
                    } else if (!!e.resurrect && (!filterGuid || e.resurrect.guid == filterGuid)) {
                        let guid = e.resurrect.guid
                        let playerName = this.guidToName.has(guid) ?
                            this.guidToName.get(guid)! :
                            guid
                        data.addXMarkLine({
                            x: this.convertTmToX(e.tm),
                            name: `${playerName} - Resurrect`,
                            symbol: `image://assets/wow/stats/res.png`,
                            colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, colors.specIdToColor(this.guidToSpecId.get(guid)!))),
                        })
                    }
                }
            }
        }

        let addBloodlustToSeries = (data: StatXYSeriesData, guid: string | undefined) => {
            if (this.showBloodlust && !!this.events) {
                for (let aura of this.events.auras) {
                    if (!dateRangeIntersects(aura.appliedTm, aura.removedTm, this.encounterStartTime, this.encounterEndTime)) {
                        continue
                    }

                    if (!!guid && guid !== aura.targetGuid) {
                        continue
                    }

                    if (wowc.BLOODLUST_SPELL_IDS.has(aura.spellId) && aura.spellId in this.spellIdNames) {
                        data.addXMarkArea({
                            start: this.convertTmToX(dateClamp(aura.appliedTm, this.encounterStartTime, this.encounterEndTime)),
                            end: this.convertTmToX(dateClamp(aura.removedTm, this.encounterStartTime, this.encounterEndTime)),
                            name: this.spellIdNames[aura.spellId]!
                        })
                    }
                }
            }
        }

        let addAurasAndSpellsToSeries = (data: StatXYSeriesData, guid: string) => {
            if (!this.events || !this.separateGraphs) {
                return
            }

            let playerName = this.guidToName.has(guid) ?
                this.guidToName.get(guid)! :
                guid

            let periodData = new StatTimePeriodData(playerName)
            if (this.showSpells) {
                let track = new StatTimePeriodTrackData('Spells')

                for (let e of this.events.spellCasts) {
                    if (e.sourceGuid !== guid || !e.success || !wowc.WOW_IMPORTANT_SPELL_IDS.has(e.spellId)) {
                        continue
                    }

                    if (!dateRangeIntersects(e.instant ? e.castFinish : e.castStart!, e.castFinish, this.encounterStartTime, this.encounterEndTime)) {
                        continue
                    }

                    let start = this.convertTmToX(dateClamp(e.instant ? e.castFinish : e.castStart!, this.encounterStartTime, this.encounterEndTime))
                    let end = this.convertTmToX(dateClamp(e.castFinish, this.encounterStartTime, this.encounterEndTime))

                    if (e.spellId in this.spellIdNames) {
                        let section = new StatTimePeriodSection(this.spellIdNames[e.spellId], start, end)
                        section.setIcon(staticClient.getWowSpellIconUrl(this.patch, e.spellId))
                        section.setColor(colorToCssString(colorFromElementTheme(this.$parent.$el, colors.spellSchoolToColor(e.spellSchool))))
                        track.addSection(section)
                    }
                }

                periodData.addTrack(track)
            }

            if (this.showAuras) {
                let track = new StatTimePeriodTrackData('Auras')

                for (let e of this.events.auras) {
                    if (e.targetGuid !== guid || !wowc.WOW_IMPORTANT_SPELL_IDS.has(e.spellId)) {
                        continue
                    }

                    if (!dateRangeIntersects(e.appliedTm, e.removedTm, this.encounterStartTime, this.encounterEndTime)) {
                        continue
                    }

                    let start = this.convertTmToX(dateClamp(e.appliedTm, this.encounterStartTime, this.encounterEndTime))
                    let end = this.convertTmToX(dateClamp(e.removedTm, this.encounterStartTime, this.encounterEndTime))

                    if (e.spellId in this.spellIdNames) {
                        let section =  new StatTimePeriodSection(this.spellIdNames[e.spellId], start, end)
                        section.setIcon(staticClient.getWowSpellIconUrl(this.patch, e.spellId))

                        if (isWowAuraBuff(e.auraType)) {
                            section.setColor(colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-top-place')))
                            section.setPreferredLayer(0)
                        } else {
                            section.setColor(colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-bottom-place')))
                            section.setPreferredLayer(1)
                        }

                        track.addSection(section)
                    }
                }

                periodData.addTrack(track)
            }

            if (periodData._tracks.length > 0) {
                data.addOverlayPeriod(periodData)
            }
        }

        for (let e of this.endpoints) {
            if (!(e in this.cachedStats)) {
                continue
            }

            if (this.aggregateStats && !this.separateGraphs) {
                let fn = (c: WowCharacter[], friendly: boolean, hasTwoTeam: boolean) : StatXYSeriesData => {
                    let data = new StatXYSeriesData(
                        [],
                        [],
                        'value',
                        'elapsedSeconds',
                        `${friendly ? 'Your Team' : 'Enemy Team'} - ${endpointToStatName(e)}`
                    )

                    let xy: Map<number, number> = new Map()
                    let allowedGuids: Set<string> = new Set(c.map((c: WowCharacter) => c.guid))

                    for (let [guid, stats] of Object.entries(this.cachedStats[e])) {
                        if (!allowedGuids.has(guid)) {
                            continue
                        }

                        let x = (<WowStatDatum[]>stats).map((ele: WowStatDatum) => ele.tm)
                        let y = (<WowStatDatum[]>stats).map((ele: WowStatDatum) => ele.value)

                        for (let i = 0; i < x.length; ++i) {
                            if (xy.has(x[i])) {
                                xy.set(x[i], y[i] + xy.get(x[i])!)
                            } else {
                                xy.set(x[i], y[i])
                            }
                        }
                    }

                    let paddedX = this.expectedXRange
                    let paddedY = this.expectedXRange.map((x: number) => {
                        if (xy.has(x)) {
                            return xy.get(x)!
                        } else {
                            return 0
                        }
                    })
                    
                    data.setXY(paddedX.map((ele: number) => {
                        // Need to account for the offset between the current encounter
                        // and the start of the match.
                        return ele + (this.encounterStartTime.getTime() - this.startTime.getTime()) / 1000.0
                    }), paddedY)

                    if (!hasTwoTeam) {
                        data.setStyle(endpointToLineStyle.get(e))
                    } else {
                        data.setSymbol(endpointToSymbol.get(e))
                        if (friendly) {
                            data.setStyle({
                                color: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-top-place'))
                            })
                        } else {
                            data.setStyle({
                                color: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-bottom-place'))
                            })
                        }
                    }

                    return data
                }

                const hasTwoTeam = this.enemyCharacters.length > 0

                let friendlyData = fn(this.friendlyCharacters, true, hasTwoTeam)
                addTimeToSeries(friendlyData)
                addEventsToSeries(friendlyData, undefined)
                addBloodlustToSeries(friendlyData, undefined)

                series.push(friendlyData)
                if (hasTwoTeam) {
                    series.push(fn(this.enemyCharacters, false, hasTwoTeam))
                }
            } else {
                for (let [guid, stats] of Object.entries(this.cachedStats[e])) {
                    if (!this.displayGuidSet.has(guid)) {
                        continue
                    }

                    let playerName = this.guidToName.has(guid) ?
                        this.guidToName.get(guid)! :
                        guid
                    let name = `${playerName}'s ${endpointToStatName(e)}`
                    let group = playerName

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
                    data.setGroup(group)
                    data.setGroupStyle(
                        colorToCssString(
                            colorFromElementTheme(
                                this.$parent.$el,
                                this.guidToTeam.get(guid) === this.friendlyTeam ? 'color-friendly' : 'color-enemy'
                            )
                        ),
                        staticClient.getWowSpecsIconUrl(this.patch, this.guidToSpecId.get(guid)!)
                    )
                    data.setSymbol(guidToSymbol.get(guid))

                    if (this.separateGraphs) {
                        data.setStyle(endpointToLineStyle.get(e))
                    } else {
                        data.setStyle(guidToLineStyle.get(guid))
                    }

                    if (e === markerEndpoint) {
                        addTimeToSeries(data)
                        addEventsToSeries(data, guid)
                        addBloodlustToSeries(data, guid)
                        addAurasAndSpellsToSeries(data, guid)
                    }

                    series.push(data)
                }
            }
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
            Math.max(250 * this.uniqueGroups.length, BASE_GRAPH_HEIGHT) :
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

    get guidToSpecId(): Map<string, number> {
        let mapping = new Map<string, number>()
        for (let m of this.matchCharacters) {
            mapping.set(m.guid, m.specId)
        }
        return mapping
    }

    get guidToTeam(): Map<string, number> {
        let mapping = new Map<string, number>()
        for (let m of this.matchCharacters) {
            mapping.set(m.guid, m.team)
        }
        return mapping
    }

    @Watch('encounterStartTime')
    @Watch('encounterEndTime')
    forceRefreshData() {
        this.cachedStats = {}
        this.refreshData()
    }

    @Watch('matchCharacters')
    @Watch('charGuid')
    resetDisplayOptions() {
        let char = this.matchCharacters.find((ele: WowCharacter) => ele.guid === this.charGuid)
        
        if (!!char) {
            this.charactersToDisplay = [char]
        } else if (this.matchCharacters.length > 0) {
            this.charactersToDisplay = [this.matchCharacters[0]]
        } else {
            this.charactersToDisplay = []
        }
    }

    refreshDataEndpoint(endpoint: string) {
        if (endpoint in this.cachedStats) {
            return
        }

        if (this.pendingEndpoints.has(endpoint)) {
            return
        }

        this.pendingEndpoints.add(endpoint)

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

    refreshDataDps() {
        if (this.showDps) {
            this.refreshDataEndpoint(DPS_ENDPOINT)
        }
    }
    
    refreshDataDamageReceived() {
        if (this.showDrps) {
            this.refreshDataEndpoint(DRPS_ENDPOINT)
        }
    }

    refreshDataHeals() {
        if (this.showHps) {
            this.refreshDataEndpoint(HPS_ENDPOINT)
        }
    }

    refreshData() {
        this.refreshDataDps()
        this.refreshDataDamageReceived()
        this.refreshDataHeals()
    }

    mounted() {
        this.resetDisplayOptions()
        this.refreshData()
    }

    redraw() {
        this.$refs.graph.graphResize()   
    }
}

</script>