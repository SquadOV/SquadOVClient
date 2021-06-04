<template>
    <div class="full-width">
        <div class="d-flex align-center">
            <v-dialog
                v-model="showSettings"
                max-width="60%"
            >
                <template v-slot:activator="{on}">
                    <v-btn
                        class="my-1"
                        v-on="on"
                        block
                        color="primary"
                    >
                        Settings
                    </v-btn>
                </template>
                
                <v-card class="full-parent-height">
                    <v-container class="full-parent-height">
                        <v-row class="full-parent-height">
                            <v-col cols="6" class="full-parent-height">
                                <v-list dense class="full-parent-height" style="overflow: auto;">
                                    <v-list-item-group
                                        v-model="showGuids"
                                        multiple
                                    >
                                        <v-list-item
                                            v-for="(c, idx) in matchCharacters"
                                            :key="`guid-${idx}`"
                                            :value="c.guid"
                                        >
                                            <template v-slot:default="{ active }">
                                                <v-list-item-action>
                                                    <v-checkbox :input-value="active"></v-checkbox>
                                                </v-list-item-action>

                                                <wow-character-display :character="c" :patch="patch">
                                                </wow-character-display>
                                            </template>
                                        </v-list-item>
                                    </v-list-item-group>
                                </v-list>
                            </v-col>

                            <v-col cols="6">
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
import Component from 'vue-class-component'
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
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
const HEIGHT_PER_LAYER = 32

@Component({
    components: {
        TimePeriodGraph,
        WowCharacterDisplay
    }
})
export default class WowSpellAnalysis extends Vue {
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

    spellIdNames: {[id: number]: string} = {}

    showGuids: string[] = []
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

    get guidSet(): Set<string> {
        return new Set(this.showGuids)
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
                    )
                    staticClient.getWowSpecsIconUrl(this.patch, c.specId)
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

    mounted() {
        this.showGuids = this.matchCharacters.map((c: WowCharacter) => c.guid)
    }

    redraw() {
        this.$refs.graph.graphResize()   
    }
}

</script>