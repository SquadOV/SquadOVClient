<template>
    <loading-container :is-loading="!events">
        <template v-slot:default="{ loading }">
            <v-sheet class="full-parent-height" v-if="!loading">
                <div class="d-flex align-center pa-4">
                    <v-select
                        dense
                        solo
                        label="Encounter"
                        :value="selectedEncounter"
                        @input="$emit('update:selectedEncounter', arguments[0])"
                        :items="encounterItems"
                        hide-details
                    >
                    </v-select>

                    <template v-if="!!selectedEncounter && hasVod">
                        <v-btn
                            @click="goToEncounterStart"
                            color="primary"
                            fab
                            small
                        >
                            <v-icon>mdi-play</v-icon>
                        </v-btn>

                        <v-btn
                            class="ml-4"
                            @click="goToEncounterEnd"
                            color="success"
                            fab
                            small
                        >
                            <v-icon>mdi-stop</v-icon>
                        </v-btn>
                    </template>
                </div>

                <v-dialog
                    v-model="showFilters"
                    max-width="60%"
                >
                    <template v-slot:activator="{on}">
                        <v-btn
                            v-on="on"
                            block
                            small
                            color="success"
                        >
                            <v-icon>
                                mdi-cog
                            </v-icon>

                            Filters
                        </v-btn>
                    </template>

                    <v-card>
                        <v-card-title>
                            Filters
                        </v-card-title>
                        <v-divider></v-divider>

                        <div class="d-flex">
                            <div>
                                <div class="d-flex justify-center align-center text-h6 font-weight-bold text-uppercase">
                                    Who
                                </div>

                                <div class="d-flex flex-wrap align-center px-4 pb-4">
                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showFriendly"
                                        label="Friendly"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showHostile"
                                        label="Hostile"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showNeutral"
                                        label="Neutral"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>
                                </div>

                                <div class="d-flex flex-wrap align-center pa-4">
                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showPlayers"
                                        label="Players"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showNpc"
                                        label="NPC"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>
                                </div>

                                <div class="d-flex flex-wrap align-center pa-4">
                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showPet"
                                        label="Pet"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showTrash"
                                        label="Trash"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showRaidBoss"
                                        label="Boss"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>
                                </div>
                            </div>

                            <v-divider vertical></v-divider>

                            <div>
                                <div class="d-flex justify-center align-center text-h6 font-weight-bold text-uppercase">
                                    What
                                </div>

                                <div class="d-flex flex-wrap align-center px-4 pb-4">
                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showDeaths"
                                        label="Deaths"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <v-checkbox
                                        class="mx-2"
                                        v-model="showResurrects"
                                        label="Resurrects"
                                        dense
                                        hide-details
                                    >
                                    </v-checkbox>
                                </div>
                            </div>
                        </div>

                        <v-card-actions>
                            <v-spacer></v-spacer>
                            <v-btn
                                color="success"
                                @click="showFilters = false"
                            >
                                Close
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>

                <v-divider></v-divider>

                <v-list class="event-list pa-0">
                    <v-list-item
                        v-for="(eve, index) in unifiedEvents"
                        :key="index"
                        :style="eventStyling(eve)"
                    >
                        <v-list-item-content class="event-time">
                            {{ eventTimeToStr(eve.tm) }}
                        </v-list-item-content>

                        <v-list-item-content class="px-4">
                            <template v-if="!!eve.death">
                                <v-icon class="event-icon mr-4" small>
                                    mdi-skull
                                </v-icon>
                                {{ eve.death.name }}
                            </template>

                            <template v-else-if="!!eve.resurrect">
                                <v-icon class="event-icon mr-4" small>
                                    mdi-heart-pulse
                                </v-icon>
                                {{ eve.resurrect.name }}
                            </template>
                        </v-list-item-content>

                        <v-list-item-action class="ml-0" v-if="!!hasVod">
                            <v-btn
                                outlined
                                fab
                                small
                                color="white"
                                @click="goToEvent(eve)"
                            >
                                <v-icon>mdi-play</v-icon>
                            </v-btn>
                        </v-list-item-action>
                    </v-list-item>
                </v-list>
            </v-sheet>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import {
    SerializedWowMatchEvents,
    WowEncounter,
    WowDeath,
    WowResurrection,
    UnifiedWowEventContainer,
    WowUserTarget
} from '@client/js/wow/events'
import {
    WowCharacter
} from '@client/js/wow/character'
import * as wowc from '@client/js/wow/constants'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { secondsToTimeString } from '@client/js/time'
import { applyFilterToCondition } from '@client/js/boolean'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        LoadingContainer
    }
})
export default class WowMatchEvents extends mixins(CommonComponent) {
    @Prop({required: true})
    events!: SerializedWowMatchEvents | null

    @Prop({required: true})
    selectedEncounter!: WowEncounter | null

    @Prop({required: true})
    startTime!: Date

    @Prop({type: Boolean, required: true})
    hasVod!: boolean

    @Prop({required: true})
    currentCharacter!: string | null

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({type: Array})
    syncUnifiedEvents!: UnifiedWowEventContainer[]

    @Prop({type: Boolean, required: true})
    isArena!: boolean

    @Prop({type: Boolean, default: false})
    useTeams!: boolean

    @Prop({default: 0})
    friendlyTeam!: number

    // Event filters
    showFilters: boolean = false
    showFriendly: boolean = true
    showHostile: boolean = true
    showNeutral: boolean = false
    showPlayers: boolean = true
    showNpc: boolean = true
    showPet: boolean = false
    showTrash: boolean = false
    showRaidBoss: boolean = true

    showDeaths: boolean = true
    showResurrects: boolean = true

    goToEncounterStart() {
        if (!this.selectedEncounter) {
            return
        }

        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.GoToEvent,
            'Encounter Start',
            this.selectedEncounter.startTm.getTime()
        )
        this.$emit('go-to-event', this.selectedEncounter.startTm, false)
    }

    goToEncounterEnd() {
        if (!this.selectedEncounter) {
            return
        }

        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.GoToEvent,
            'Encounter End',
            this.selectedEncounter.endTm.getTime()
        )
        this.$emit('go-to-event', this.selectedEncounter.endTm, false)
    }

    goToEvent(e: UnifiedWowEventContainer) {
        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.GoToEvent,
            !!e.death ? 'Death' :
                !!e.resurrect ? 'Resurrect' : '',
            e.tm.getTime(),
        )
        this.$emit('go-to-event', e.tm, true)
    }

    obtainUserEventHighlightColor(e: WowUserTarget): string {
        if (e.guid == this.currentCharacter) {
            return 'color-self'
        } else if (this.friendlyGuidSet.has(e.guid)) {
            return 'color-friendly'
        }
        return 'color-enemy'
    }

    eventStyling(e : UnifiedWowEventContainer) : any {
        let borderHighlightColor: string = 'color-transparent'

        if (!!e.death) {
            borderHighlightColor = this.obtainUserEventHighlightColor(e.death)
        } else if (!!e.resurrect) {
            borderHighlightColor = this.obtainUserEventHighlightColor(e.resurrect)
        }

        let style: any = {
            'border-left': `5px solid var(--${borderHighlightColor})`
        }
        return style
    }

    eventTimeToStr(tm : Date): string {
        let secs = (tm.getTime() - this.startTime.getTime()) / 1000
        return secondsToTimeString(secs)
    }

    get friendlyGuidSet(): Set<string> {
        return new Set(this.matchCharacters.filter((ele: WowCharacter) => {
            return !this.useTeams || (ele.team === this.friendlyTeam)
        }).map((ele: WowCharacter) => ele.guid))
    }

    get encounterNameSet(): Set<string> {
        let ret = new Set<string>(wowc.WOW_OVERRIDE_BOSS_NAMES)

        if (!!this.events) {
            for (let enc of this.events.encounters) {
                ret.add(enc.encounterName)
            }
        }

        return ret
    }

    @Watch('unifiedEvents')
    propagateUnifiedEvents() {
        this.$emit('update:syncUnifiedEvents', this.unifiedEvents)
    }

    mounted() {
        this.propagateUnifiedEvents()
    }

    checkFilterPass(ele: WowUserTarget): boolean {
        if (!wowc.objectHasType(ele.flags)) {
            return false
        }

        let friendly = wowc.isObjectFriendly(ele.flags)
        let hostile = wowc.isObjectHostile(ele.flags)
        let neutral = wowc.isObjectNeutral(ele.flags)
        let player = wowc.isObjectPlayer(ele.flags)
        let npc = wowc.isObjectNpc(ele.flags)
        let pet = (friendly || neutral || (this.isArena && hostile)) && wowc.isObjectPet(ele.flags)
        let trash = hostile && npc && !this.encounterNameSet.has(ele.name)
        let boss = hostile && npc && this.encounterNameSet.has(ele.name)
        return applyFilterToCondition(friendly, this.showFriendly)
            && applyFilterToCondition(hostile, this.showHostile)
            && applyFilterToCondition(neutral, this.showNeutral)
            && applyFilterToCondition(player, this.showPlayers)
            && applyFilterToCondition(npc, this.showNpc)
            && applyFilterToCondition(pet, this.showPet)
            && applyFilterToCondition(trash, this.showTrash)
            && applyFilterToCondition(boss, this.showRaidBoss)
    }

    mergeUnifiedEventContainers(...args: UnifiedWowEventContainer[][]): UnifiedWowEventContainer[] {
        let unified: UnifiedWowEventContainer[] = []
        while (args.some((ele: UnifiedWowEventContainer[]) => ele.length > 0)) {
            let selectedIndex: number = args.length
            let minTime: Date = new Date()

            for (let i = 0; i < args.length; ++i) {
                if (args[i].length == 0) {
                    continue
                }

                if (args[i][0].tm <= minTime) {
                    selectedIndex = i
                    minTime = args[i][0].tm
                }
            }

            if (selectedIndex == args.length) {
                console.warn('Failed to merge event containers???')
                break
            }

            unified.push(args[selectedIndex].shift()!)
        }
        return unified
    }

    get unifiedEvents(): UnifiedWowEventContainer[] {
        if (!this.events) {
            return []
        }

        let retEvents: UnifiedWowEventContainer[] = this.mergeUnifiedEventContainers(
            this.events.deaths
                .filter((ele: WowDeath) => {
                    if (!this.selectedEncounter) {
                        return true
                    }

                    return ele.tm >= this.selectedEncounter!.startTm && ele.tm <= this.selectedEncounter!.endTm
                })
                .map((ele: WowDeath) => ({
                    death: ele,
                    tm: ele.tm
                })),
            this.events.resurrections
                .filter((ele: WowResurrection) => {
                    if (!this.selectedEncounter) {
                        return true
                    }

                    return ele.tm >= this.selectedEncounter!.startTm && ele.tm <= this.selectedEncounter!.endTm
                })
                .map((ele: WowResurrection) => ({
                    resurrect: ele,
                    tm: ele.tm
                }))
        )

        return retEvents.filter((ele: UnifiedWowEventContainer) => {
            // FILTERS MUST BE HERE. I'm not sure why putting the filter checks in the
            // .filter of the arguments in the mergeUnifiedEventContainers call doesn't work.
            if (!!ele.death) {
                return this.showDeaths && this.checkFilterPass(ele.death)
            } else if (!!ele.resurrect) {
                return this.showResurrects && this.checkFilterPass(ele.resurrect)
            }
            return true
        })
    }

    get encounterItems(): any[] {
        if (!this.events) {
            return []
        }

        let retEvents : any[] = []
        retEvents.push({
            text: 'All Encounters',
            value: null
        })

        let encounterCount: Map<string, number> = new Map()
        for (let enc of this.events.encounters) {
            if (encounterCount.has(enc.encounterName)) {
                encounterCount.set(enc.encounterName, encounterCount.get(enc.encounterName)! + 1)
            } else {
                encounterCount.set(enc.encounterName, 1)
            }
            let num = encounterCount.get(enc.encounterName)!
            let suffix = (num > 1) ? ` (#${num})` : ''

            retEvents.push({
                text: `${enc.encounterName}${suffix}`,
                value: enc
            })
        }

        return retEvents
    }
}

</script>

<style scoped>

.event-list {
    overflow-y: auto;
    height: calc(100% - 70px - 28px);
}

.event-time {
    flex: 0 0 auto !important;
}

.event-icon {
    flex-grow: 0;
    flex-basis: 0;
}

</style>