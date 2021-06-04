<template>
    <div>
        <v-sheet class="full-parent-height" v-if="hasRound">
            <div class="d-flex align-center pa-4">
                <span class="text-h5">
                    Round {{ round._r.roundNum + 1 }}
                </span>

                <v-spacer></v-spacer>
                
                <v-btn 
                    @click="goToBuy"
                    color="success"
                    fab
                    small
                    v-if="!!buyTime"
                >
                    <v-icon>mdi-currency-usd</v-icon>
                </v-btn>

                <v-btn
                    class="ml-4"
                    @click="goToPlay"
                    color="primary"
                    fab
                    small
                    v-if="!!playTime"
                >
                    <v-icon>mdi-play</v-icon>
                </v-btn>
            </div>
            <v-divider></v-divider>

            <v-list class="event-list pa-0">
                <v-list-item
                    v-for="(eve, index) in allEvents"
                    :key="index"
                    :style="eventStyling(eve)"
                >
                    <v-list-item-content class="event-time">
                        {{ formatRoundTime(eve.roundTimeMs) }}
                    </v-list-item-content>

                    <v-list-item-content class="px-4">
                        <div class="d-flex align-center justify-space-around">
                            <template v-if="!!eve.kill">
                                <valorant-agent-icon
                                    v-if="!!eve.kill.killer"
                                    :agent="eve.kill.killer._p.characterId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <valorant-weapon-ability-icon
                                    :agent="eve.kill.killer._p.characterId"
                                    :equip-type="eve.kill._k.finishingDamage.damageType"
                                    :equip-id="eve.kill._k.finishingDamage.damageItem"
                                    :max-height="40"
                                    :max-width="150"
                                >
                                </valorant-weapon-ability-icon>

                                <valorant-agent-icon
                                    :agent="eve.kill.victim._p.characterId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>
                            </template>

                            <template v-if="!!eve.plant">
                                <valorant-agent-icon
                                    :agent="eve.plant._p.characterId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <span class="font-weight-bold">Bomb Planted</span>

                                <v-img
                                    :src="$root.generateAssetUri('assets/valorant/bomb/plant.png')"
                                    class="flex-grow-0"
                                >
                                </v-img>
                            </template>

                            <template v-if="!!eve.defuse">
                                <valorant-agent-icon
                                    :agent="eve.defuse._p.characterId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <span class="font-weight-bold">Bomb Defused</span>

                                <v-img
                                    :src="$root.generateAssetUri('assets/valorant/bomb/defuse.png')"
                                    class="flex-grow-0"
                                >
                                </v-img>
                            </template>
                        </div>
                    </v-list-item-content>

                    <v-list-item-action class="ml-0" v-if="!!playTime">
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

        <v-row justify="center" align="center" v-else>
            <span class="text-h4">Invalid Round Data</span>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { ValorantMatchPlayerMatchMetadata } from '@client/js/valorant/valorant_matches'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchRoundWrapper,
    ValorantMatchPlayerWrapper,
    ValorantMatchKillWrapper
} from '@client/js/valorant/valorant_matches_parsed'
import { Color, colorFromElementTheme } from '@client/js/color'
import { formatRoundTime } from '@client/js/valorant/valorant_utility'
import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantWeaponAbilityIcon from '@client/vue/utility/valorant/ValorantWeaponAbilityIcon.vue'

interface RoundEvent {
    roundTimeMs: number
    kill? : ValorantMatchKillWrapper
    plant? : ValorantMatchPlayerWrapper
    defuse? : ValorantMatchPlayerWrapper
}

@Component({
    components: {
        ValorantAgentIcon,
        ValorantWeaponAbilityIcon
    }
})
export default class ValorantRoundEvents extends Vue {
    formatRoundTime : any = formatRoundTime

    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop({required: true})
    round! : ValorantMatchRoundWrapper | null

    @Prop()
    currentPlayer! : ValorantMatchPlayerWrapper | null

    @Prop({default: null})
    metadata!: ValorantMatchPlayerMatchMetadata | null

    @Prop({type: Boolean, default: false})
    forceDisableGoToEvent! : boolean

    eventPlayer(e: RoundEvent) : ValorantMatchPlayerWrapper | null {
        if (!!e.kill) {
            return e.kill.killer
        } else if (!!e.plant) {
            return e.plant
        } else if (!!e.defuse) {
            return e.defuse
        }
        return null
    }

    altEventPlayer(e: RoundEvent) : ValorantMatchPlayerWrapper | null {
        if (!!e.kill) {
            return e.kill.victim
        }
        return null
    }

    eventStyling(e : RoundEvent) : any {
        // IF PLAYER SPECIFIED:
        //  - Green background accent for same team
        //  - Red background accent for opposing team
        //  - Left border yellow/gold accent for if current player
        // IF PLAYER NOT SPECIFIED:
        //  - Blue background accent for blue team
        //  - Red background accent for red team
        let eventPlayer = this.eventPlayer(e)
        if (!eventPlayer) {
            return {}
        }
        let altPlayer = this.altEventPlayer(e)

        let color : string
        let isSelf = false
        if (!!this.currentPlayer) {
            if (eventPlayer._p.teamId == this.currentPlayer._p.teamId) {
                color = 'color-friendly'
            } else {
                color = 'color-enemy'
            }
            isSelf = (eventPlayer._p.puuid == this.currentPlayer._p.puuid) ||
                (altPlayer?._p.puuid == this.currentPlayer._p.puuid)
        } else {
            if (eventPlayer._p.teamId == 'Blue') {
                color = 'color-blue-team'
            } else if (eventPlayer._p.teamId == 'Red') {
                color = 'color-red-team'
            } else {
                color = 'color-neutral'
            }
        }

        let rgb: Color = colorFromElementTheme(this.$parent.$el, color)
        let style : any = {
            'background': `linear-gradient(90deg, rgba(${rgb.r},${rgb.g},${rgb.b},0.0) 70%, rgba(${rgb.r},${rgb.g},${rgb.b},0.5) 100%)`
        }

        if (isSelf) {
            style['border-left'] = '5px solid var(--color-self)'
        }

        return style
    }

    get allEvents() : RoundEvent[] {
        if (!this.round) {
            return []
        }

        let events : RoundEvent[] = []
        if (!!this.round._r.plantRoundTime && !!this.round._r.bombPlanter) {
            events.push({
                roundTimeMs: this.round._r.plantRoundTime,
                plant: this.match.getPlayer(this.round._r.bombPlanter),
            })
        }

        if (!!this.round._r.defuseRoundTime  && !!this.round._r.bombDefuser) {
            events.push({
                roundTimeMs: this.round._r.defuseRoundTime,
                defuse: this.match.getPlayer(this.round._r.bombDefuser),
            })
        }

        for (let k of this.round.kills) {
            events.push({
                roundTimeMs: k._k.timeSinceRoundStartMillis,
                kill: k,
            })
        }

        events.sort((a : RoundEvent, b : RoundEvent) => {
            return a.roundTimeMs - b.roundTimeMs
        })

        return events
    }

    get hasRound() : boolean {
        return !!this.round
    }

    get buyTime() : Date | null | undefined {
        return this.metadata?.rounds[this.round!._r.roundNum]?.buyTime
    }

    get playTime() : Date| null | undefined {
        return this.metadata?.rounds[this.round!._r.roundNum]?.roundTime
    }

    goToPlay() {
        this.$emit('go-to-event', this.playTime!)
    }

    goToBuy() {
        this.$emit('go-to-event', this.buyTime!)
    }

    goToEvent(e : RoundEvent) {
        // Offset so we can actually see the action happen.
        this.$emit('go-to-event', new Date(this.playTime!.getTime() + e.roundTimeMs))
    }
}

</script>

<style scoped>

.event-list {
    overflow-y: auto;
    height: calc(100% - 73px);
}

.event-time {
    flex: 0 0 auto !important;
}

</style>