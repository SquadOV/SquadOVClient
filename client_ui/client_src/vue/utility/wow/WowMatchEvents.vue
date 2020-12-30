<template>
    <loading-container :is-loading="!events">
        <template v-slot:default="{ loading }">
            <v-sheet class="full-parent-height" v-if="!loading">
                <div class="d-flex align-center pa-4">
                    <v-select
                        dense
                        solo
                        label="Encounter"
                        v-model="selectedEncounter"
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

                        <div class="d-flex flex-wrap align-center pa-4">
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

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import {
    SerializedWowMatchEvents,
    WowEncounter,
    WowDeath,
    UnifiedWowEventContainer
} from '@client/js/wow/events'
import {
    WowCharacter
} from '@client/js/wow/character'
import * as colors from '@client/js/wow/colors'
import * as wowc from '@client/js/wow/constants'
import { colorToCssString } from '@client/js/color'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { secondsToTimeString } from '@client/js/time'
import { applyFilterToCondition } from '@client/js/boolean'

@Component({
    components: {
        LoadingContainer
    }
})
export default class WowMatchEvents extends Vue {
    @Prop({required: true})
    events!: SerializedWowMatchEvents | null
    selectedEncounter: WowEncounter | null = null

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

    goToEncounterStart() {
        if (!this.selectedEncounter) {
            return
        }
        this.$emit('go-to-event', this.selectedEncounter.startTm)
    }

    goToEncounterEnd() {
        if (!this.selectedEncounter) {
            return
        }
        this.$emit('go-to-event', this.selectedEncounter.endTm)
    }

    goToEvent(e: UnifiedWowEventContainer) {
        this.$emit('go-to-event', e.tm)
    }

    eventStyling(e : UnifiedWowEventContainer) : any {
        let borderHighlightColor: string = 'transparent'

        if (!!e.death) {
            if (e.death.guid == this.currentCharacter) {
                borderHighlightColor = colorToCssString(colors.getSelfColor())
            } else if (this.friendlyGuidSet.has(e.death.guid)) {
                borderHighlightColor = colorToCssString(colors.getSuccessColor())
            } else {
                borderHighlightColor = colorToCssString(colors.getFailureColor())
            }
        }

        let style: any = {
            'border-left': `5px solid ${borderHighlightColor}`
        }
        return style
    }

    eventTimeToStr(tm : Date): string {
        let secs = (tm.getTime() - this.startTime.getTime()) / 1000
        return secondsToTimeString(secs)
    }

    get friendlyGuidSet(): Set<string> {
        return new Set(this.matchCharacters.map((ele: WowCharacter) => ele.guid))
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

    get unifiedEvents(): UnifiedWowEventContainer[] {
        if (!this.events) {
            return []
        }

        let retEvents: UnifiedWowEventContainer[] = []
        if (!!this.selectedEncounter) {
            retEvents = this.events.deaths
                .filter((ele: WowDeath) => {
                    return ele.tm >= this.selectedEncounter!.startTm && ele.tm <= this.selectedEncounter!.endTm
                })
                .map((ele: WowDeath) => ({
                    death: ele,
                    tm: ele.tm
                }
            ))
        } else {
            retEvents = this.events.deaths.map((ele: WowDeath) => ({
                death: ele,
                tm: ele.tm
            }))
        }

        return retEvents.filter((ele: UnifiedWowEventContainer) => {
            if (!!ele.death) {
                let friendly = wowc.isObjectFriendly(ele.death.flags)
                let hostile = wowc.isObjectHostile(ele.death.flags)
                let neutral = wowc.isObjectNeutral(ele.death.flags)
                let player = wowc.isObjectPlayer(ele.death.flags)
                let npc = wowc.isObjectNpc(ele.death.flags)
                let pet = (friendly || neutral) && wowc.isObjectPet(ele.death.flags)
                let trash = hostile && npc && !this.encounterNameSet.has(ele.death.name)
                let boss = hostile && npc && this.encounterNameSet.has(ele.death.name)
                return applyFilterToCondition(friendly, this.showFriendly)
                    && applyFilterToCondition(hostile, this.showHostile)
                    && applyFilterToCondition(neutral, this.showNeutral)
                    && applyFilterToCondition(player, this.showPlayers)
                    && applyFilterToCondition(npc, this.showNpc)
                    && applyFilterToCondition(pet, this.showPet)
                    && applyFilterToCondition(trash, this.showTrash)
                    && applyFilterToCondition(boss, this.showRaidBoss)
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