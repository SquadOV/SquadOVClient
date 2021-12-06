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
                
                <v-card class="full-parent-height">
                    <v-container class="full-parent-height">
                        <v-row class="full-parent-height">
                            <v-col cols="12">
                                <div class="text-overline">
                                    General
                                </div>
                                <v-divider></v-divider>
                                <div class="d-flex flex-wrap justify-space-around">
                                    <v-checkbox
                                        v-model="onlyImportant"
                                        label="Only Important"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        v-model="showTime"
                                        label="Show Time"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        v-model="showEvents"
                                        label="Show Events"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>
                                </div>

                                <div class="text-overline">
                                    Spells
                                </div>
                                <v-divider></v-divider>
                                <div class="d-flex flex-wrap justify-space-around">
                                    <v-checkbox
                                        v-model="showSpells"
                                        label="Show Spells"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        v-model="showFailed"
                                        label="Show Failed Casts"
                                        dense
                                        hide-details
                                        :disabled="!showSpells"
                                    >
                                    </v-checkbox>
                                </div>

                                <div class="text-overline">
                                    Auras
                                </div>
                                <v-divider></v-divider>
                                <div class="d-flex flex-wrap justify-space-around">
                                    <v-checkbox
                                        v-model="showAuras"
                                        label="Show Auras"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        v-model="showBuffs"
                                        label="Show Buffs"
                                        dense
                                        hide-details
                                        :disabled="!showAuras"
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        v-model="showDebuffs"
                                        label="Show Debuffs"
                                        dense
                                        hide-details
                                        :disabled="!showAuras"
                                    >
                                    </v-checkbox>
                                </div>
                            </v-col>
                        </v-row>
                    </v-container>
                </v-card>
            </v-dialog>
        </div>

        <div class="full-width" :style="graphStyle">
            <time-period-graph
                ref="graph"
                :data="activeData"
                :forced-min-x="encounterMinX"
                :forced-max-x="encounterMaxX"
                :layer-height="HEIGHT_PER_LAYER"
                @graphclick="handleClick"
            >
            </time-period-graph>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { StatTimePeriodData, StatTimePeriodTrackData, StatTimePeriodSection } from '@client/js/stats/periodData'
import { wowCache } from '@client/js/wow/staticCache'
import { staticClient } from '@client/js/staticData'
import * as wowc from '@client/js/wow/constants'
import * as colors from '@client/js/wow/colors'
import { colorToCssString, colorAToCssString, colorFromElementTheme } from '@client/js/color'
import TimePeriodGraph from '@client/vue/stats/TimePeriodGraph.vue'
import {
    dateRangeIntersects,
    dateClamp
} from '@client/js/time'
import {
    WowCharacter,
    specIdToClassId,
} from '@client/js/wow/character'
import {
    UnifiedWowEventContainer,
    SerializedWowMatchEvents,
    isWowAuraBuff,
} from '@client/js/wow/events'
import WowCharacterChooser from '@client/vue/utility/wow/WowCharacterChooser.vue'
import CommonComponent from '@client/vue/CommonComponent'

const HEIGHT_PER_LAYER = 32

@Component({
    components: {
        TimePeriodGraph,
        WowCharacterChooser
    }
})
export default class WowSpellAnalysis extends mixins(CommonComponent) {
    HEIGHT_PER_LAYER: number = HEIGHT_PER_LAYER

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

    spellIdNames: {[id: number]: string} = {}

    charactersToDisplay: WowCharacter[] = []
    showSettings: boolean = false
    onlyImportant: boolean = false
    showFailed: boolean = false
    showTime: boolean = true
    showEvents: boolean = true
    showSpells: boolean = true
    showAuras: boolean = true
    showBuffs: boolean = true
    showDebuffs: boolean = true

    $refs!: {
        graph: TimePeriodGraph
    }

    @Watch('currentTime')
    adjustSlidingWindowBasedOnTime() {
        if (!this.currentTime) {
            return
        }
        // When we go outside of the short window of time that we're actually looking at in the graph
        // we want to adjust the sliding window so that the 'currentTime' is included in the window.
        let x = this.convertTmToX(this.currentTime)
        this.$refs.graph.bringIntoView(x)
    }

    get guidSet(): Set<string> {
        return new Set(this.charactersToDisplay.map((ele: WowCharacter) => ele.guid))
    }

    get activeData(): StatTimePeriodData[] {
        let data: StatTimePeriodData[] = []

        if (!!this.events) {
            let spellIdsToCache: Set<number> = new Set()
            for (let sc of this.events.spellCasts) {
                spellIdsToCache.add(sc.spellId)
            }

            for (let aura of this.events.auras) {
                spellIdsToCache.add(aura.spellId)
            }

            wowCache.getCache(this.patch).bulkGetSpellNames(Array.from(spellIdsToCache)).then((resp: Map<number, string>) => {
                for (let [key, value] of resp) {
                    Vue.set(this.spellIdNames, key, value)
                }
            })

            for (let c of this.matchCharacters) {
                if (!this.guidSet.has(c.guid)) {
                    continue
                }

                let d = new StatTimePeriodData(c.name)
                if (this.showSpells) {
                    let track = new StatTimePeriodTrackData('Spells')
                    for (let e of this.events.spellCasts) {
                        if (e.sourceGuid !== c.guid) {
                            continue
                        }

                        if (!dateRangeIntersects(e.instant ? e.castFinish : e.castStart!, e.castFinish, this.encounterStartTime, this.encounterEndTime)) {
                            continue
                        }

                        if (this.onlyImportant && !wowc.WOW_IMPORTANT_SPELL_IDS.has(e.spellId)) {
                            continue
                        }

                        if (!this.showFailed && !e.success) {
                            continue
                        }

                        let start = this.convertTmToX(dateClamp(e.instant ? e.castFinish : e.castStart!, this.encounterStartTime, this.encounterEndTime))
                        let end = this.convertTmToX(dateClamp(e.castFinish, this.encounterStartTime, this.encounterEndTime))

                        if (e.spellId in this.spellIdNames) {
                            let section = new StatTimePeriodSection(this.spellIdNames[e.spellId], start, end)
                            section.setIcon(staticClient.getWowSpellIconUrl(this.patch, e.spellId))
                            section.setColor(colorAToCssString({
                                ...colorFromElementTheme(this.$parent.$el, colors.spellSchoolToColor(e.spellSchool)),
                                a: e.success ? 255 : 127,
                            }))
                            section.setIconOpacity(e.success ? 1 : 0.2)
                            if (e.success) {
                                section.setPreferredLayer(0)
                            } else {
                                section.setPreferredLayer(1)
                            }
                            track.addSection(section)
                        }
                    }
                    d.addTrack(track)
                }

                if (this.showAuras) {
                    let track = new StatTimePeriodTrackData('Auras')
                    for (let e of this.events.auras) {
                        if (e.targetGuid !== c.guid) {
                            continue
                        }

                        if (!dateRangeIntersects(e.appliedTm, e.removedTm, this.encounterStartTime, this.encounterEndTime)) {
                            continue
                        }

                        if (!this.showBuffs && isWowAuraBuff(e.auraType)) {
                            continue
                        }

                        if (!this.showDebuffs && !isWowAuraBuff(e.auraType)) {
                            continue
                        }

                        if (this.onlyImportant && !wowc.WOW_IMPORTANT_SPELL_IDS.has(e.spellId)) {
                            continue
                        }

                        let start = this.convertTmToX(dateClamp(e.appliedTm, this.encounterStartTime, this.encounterEndTime))
                        let end = this.convertTmToX(dateClamp(e.removedTm, this.encounterStartTime, this.encounterEndTime))

                        if (e.spellId in this.spellIdNames) {
                            let section = new StatTimePeriodSection(this.spellIdNames[e.spellId], start, end)
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
                    d.addTrack(track)
                }

                if (d._tracks.length == 0) {
                    continue
                }

                d.setGroup(c.name)
                d.setGroupStyle(
                    colorToCssString(
                        colorFromElementTheme(
                            this.$parent.$el,
                            c.team === this.friendlyTeam ? 'color-top-place' : 'color-bottom-place'
                        )
                    ),
                    c.specId > 0 ? staticClient.getWowSpecsIconUrl(this.patch, c.specId) :
                        !!c.classId ? staticClient.getWowClassIconUrl(this.patch, c.classId) : ''
                )

                if (!!this.currentTime && this.showTime) {
                    d.addXMarkLine({
                        x: this.convertTmToX(this.currentTime),
                        name: '',
                        symbol: 'none',
                        colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-self')),
                    })
                }

                if (this.showEvents) {
                    for (let e of this.unifiedEvents) {
                        if (!!e.death && e.death.guid == c.guid) {
                            d.addXMarkLine({
                                x: this.convertTmToX(e.tm),
                                name: `Death`,
                                symbol: `image://assets/wow/stats/skull.png`,
                                colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, colors.specIdToColor(c.specId))),
                            })
                        } else if (!!e.resurrect && e.resurrect.guid == c.guid) {
                            d.addXMarkLine({
                                x: this.convertTmToX(e.tm),
                                name: `Resurrect`,
                                symbol: `image://assets/wow/stats/res.png`,
                                colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, colors.specIdToColor(c.specId))),
                            })
                        }
                    }
                }

                data.push(d)
            }
        }

        return data
    }

    handleClick(evt: {gridIndex: number, pts: number[]}) {
        // Relative to startTime because we're the way the graph is setup, the time along
        // the X axis is still going to be relative to the absolute start time of the entire
        // match and what we're receiving in pts is just the (x, y) coordinates effectively.
        let dt = new Date(this.startTime.getTime() + evt.pts[0] * 1000.0)
        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.GoToTimeline,
            'Spell',
            dt.getTime()
        )
        this.$emit('go-to-time', dt)
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

    get graphStyle(): any {
        let height = 100
        for (let g of this.activeData) {
            let layerHeight = Math.max(g.numDisplayLayers * HEIGHT_PER_LAYER, 150)
            height += (layerHeight + 40)
        }

        return {
            'height': `${height}px`
        }
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

        this.adjustSlidingWindowBasedOnTime()
    }

    mounted() {
        this.resetDisplayOptions()
    }

    redraw() {
        this.$refs.graph.graphResize()   
    }
}

</script>