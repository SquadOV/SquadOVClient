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
                    v-if="hasBuyTime"
                >
                    <v-icon>mdi-currency-usd</v-icon>
                </v-btn>

                <v-btn
                    class="ml-4"
                    @click="goToPlay"
                    color="primary"
                    fab
                    small
                    v-if="hasPlayTime"
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
                        <div class="d-flex align-center justify-space-between">
                            <template v-if="!!eve.kill">
                                <valorant-agent-icon
                                    :agent="eve.kill.killer._p.agentId"
                                    :patch="match._details.patchId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <valorant-weapon-ability-icon
                                    :agent="eve.kill.killer._p.agentId"
                                    :patch="match._details.patchId"
                                    :equip-type="eve.kill._k.damageType"
                                    :equip-id="eve.kill._k.damageItem"
                                    :max-height="40"
                                    :max-width="150"
                                >
                                </valorant-weapon-ability-icon>

                                <valorant-agent-icon
                                    :agent="eve.kill.victim._p.agentId"
                                    :patch="match._details.patchId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>
                            </template>

                            <template v-if="!!eve.plant">
                                <valorant-agent-icon
                                    :agent="eve.plant._p.agentId"
                                    :patch="match._details.patchId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <span class="font-weight-bold">Bomb Planted</span>

                                <v-img
                                    src="assets/valorant/bomb/plant.png"
                                    class="flex-grow-0"
                                >
                                </v-img>
                            </template>

                            <template v-if="!!eve.defuse">
                                <valorant-agent-icon
                                    :agent="eve.defuse._p.agentId"
                                    :patch="match._details.patchId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <span class="font-weight-bold">Bomb Defused</span>

                                <v-img
                                    src="assets/valorant/bomb/defuse.png"
                                    class="flex-grow-0"
                                >
                                </v-img>
                            </template>
                        </div>
                    </v-list-item-content>

                    <v-list-item-action class="ml-0" v-if="hasPlayTime">
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
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchRoundWrapper,
    ValorantMatchPlayerWrapper,
    ValorantMatchKillWrapper
} from '@client/js/valorant/valorant_matches_parsed'
import {
    getBlueTeamColor,
    getRedTeamColor,
    getSameTeamColor
} from '@client/js/valorant/valorant_colors'
import { Color } from '@client/js/color'
import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantWeaponAbilityIcon from '@client/vue/utility/valorant/ValorantWeaponAbilityIcon.vue'

interface RoundEvent {
    roundTimeMs: number
    kill? : ValorantMatchKillWrapper
    plant? : ValorantMatchPlayerWrapper
    defuse? : ValorantMatchPlayerWrapper
}

const offsetMs = 1500

@Component({
    components: {
        ValorantAgentIcon,
        ValorantWeaponAbilityIcon
    }
})
export default class ValorantRoundEvents extends Vue {
    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop({required: true})
    round! : ValorantMatchRoundWrapper | null

    @Prop()
    currentPlayer! : ValorantMatchPlayerWrapper | null

    @Prop({type: Boolean, default: false})
    forceDisableGoToEvent! : boolean

    formatRoundTime(ms : number) : string {
        const totalSeconds = Math.floor(ms / 1000.0)
        const minutes = `${Math.floor(totalSeconds / 60.0)}`
        const seconds = `${totalSeconds % 60}`.padStart(2, '0')
        return `${minutes}:${seconds}`
    }

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

        let color : Color = { r : 0, g : 0, b : 0}
        let isSelf = false
        if (!!this.currentPlayer) {
            if (eventPlayer._p.teamId == this.currentPlayer._p.teamId) {
                color = getSameTeamColor()
            } else {
                color = getRedTeamColor()
            }
            isSelf = (eventPlayer._p.puuid == this.currentPlayer._p.puuid) ||
                (altPlayer?._p.puuid == this.currentPlayer._p.puuid)
        } else {
            if (eventPlayer._p.teamId == 'Blue') {
                color = getBlueTeamColor()
            } else if (eventPlayer._p.teamId == 'Red') {
                color = getRedTeamColor()
            }
        }

        
        let style : any = {
            'background': `linear-gradient(90deg, rgba(${color.r},${color.g},${color.b},0.0) 70%, rgba(${color.r},${color.g},${color.b},0.5) 100%)`
        }

        if (isSelf) {
            style['border-left'] = '5px solid #FFD700'
        }

        return style
    }

    get allEvents() : RoundEvent[] {
        if (!this.round) {
            return []
        }

        let events : RoundEvent[] = []
        if (!!this.round._r.plantRoundTime && !!this.round._r.planter) {
            events.push({
                roundTimeMs: this.round._r.plantRoundTime,
                plant: this.match.getPlayer(this.round._r.planter),
            })
        }

        if (!!this.round._r.defuseRoundTime  && !!this.round._r.defuser) {
            events.push({
                roundTimeMs: this.round._r.defuseRoundTime,
                defuse: this.match.getPlayer(this.round._r.defuser),
            })
        }

        for (let k of this.round.kills) {
            events.push({
                roundTimeMs: k._k.roundTime,
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

    get hasBuyTime() : boolean {
        return !!this.round!._r.startBuyTime && !this.forceDisableGoToEvent
    }

    get hasPlayTime() : boolean {
        return !!this.round!._r.startPlayTime && !this.forceDisableGoToEvent
    }

    goToPlay() {
        this.$emit('go-to-event', this.round!._r.startPlayTime!)
    }

    goToBuy() {
        this.$emit('go-to-event', this.round!._r.startBuyTime!)
    }

    goToEvent(e : RoundEvent) {
        // Offset so we can actually see the action happen.
        this.$emit('go-to-event', new Date(this.round!._r.startPlayTime!.getTime() + e.roundTimeMs - offsetMs))
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