<template>
    <div>
        <v-sheet class="full-parent-height" v-if="!!roundData">
            <div class="d-flex align-center pa-4">
                <csgo-team-icon
                    :team="team"
                    :width-height="32"
                    class="mr-2"
                >
                </csgo-team-icon>

                <span class="text-h5">
                    Round {{ currentRound + 1 }}
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

                <v-btn
                    class="ml-4"
                    @click="goToStop"
                    color="error"
                    fab
                    small
                    v-if="!!stopTime"
                >
                    <v-icon>mdi-stop</v-icon>
                </v-btn>
            </div>
            <v-divider></v-divider>

            <v-list class="event-list pa-0">
                <v-list-item
                    v-for="(eve, index) in allEvents"
                    :key="index"
                    :style="eventStyling(eve)"
                >
                    <v-list-item-content class="event-time flex-grow-0 flex-shrink-1">
                        {{ millisecondsToTimeString(eve.roundTimeMs) }}
                    </v-list-item-content>

                    <v-list-item-content class="px-4">
                        <div class="d-flex align-center justify-space-around">
                            <template v-if="!!eve.kill">
                                <csgo-round-kill-display
                                    :match="match"
                                    :kill="eve.kill"
                                    :height="32"
                                    :round-data="roundData"
                                    :match-user-id="matchUserId"
                                >
                                </csgo-round-kill-display>
                            </template>

                            <template v-if="!!eve.plant">
                                <steam-account-display
                                    v-if="eve.plant.user !== null"
                                    :width="32"
                                    :height="32"
                                    :account="match.steamAccount(match.userIdToSteamId(eve.plant.user))"
                                    :style="userStyling(eve.plant.user)"
                                >
                                </steam-account-display>

                                <span class="font-weight-bold" v-if="eve.plant.site !== null">Bomb Planted at {{ bombSiteToString(eve.plant.site) }}</span>
                                <span class="font-weight-bold" v-else>Bomb Planted</span>

                                <v-img
                                    class="ml-2"
                                    :height="32"
                                    :max-height="32"
                                    :width="27"
                                    :max-width="27"
                                    :src="$root.generateAssetUri('assets/csgo/misc/icon-c4.png')"
                                    contain
                                >
                                </v-img>
                            </template>

                            <template v-if="!!eve.defuse">
                                <steam-account-display
                                    v-if="eve.defuse.user !== null"
                                    :width="32"
                                    :height="32"
                                    :account="match.steamAccount(match.userIdToSteamId(eve.defuse.user))"
                                    :style="userStyling(eve.defuse.user)"
                                >
                                </steam-account-display>

                                <span class="font-weight-bold">Bomb Defused</span>

                                <v-img
                                    class="ml-2"
                                    :height="32"
                                    :max-height="32"
                                    :width="36"
                                    :max-width="36"
                                    :src="$root.generateAssetUri('assets/csgo/misc/icon-defuser.png')"
                                    contain
                                >
                                </v-img>
                            </template>

                            <template v-if="!!eve.explode">
                                <span class="font-weight-bold">Bomb Exploded</span>

                                <v-icon class="ml-2">
                                    mdi-nuke
                                </v-icon>
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
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoFullMatchDataWrapper, CsgoEventRoundWrapper} from '@client/js/csgo/match'
import { CsgoBombSite, CsgoRoundKill, CsgoTeam, bombSiteToString } from '@client/js/csgo/events'
import { millisecondsToTimeString } from '@client/js/time'
import { Color, getGenericWinColor, getGenericLossColor, getGenericFirstPlaceColor, colorToCssString } from '@client/js/color'
import CsgoTeamIcon from '@client/vue/utility/csgo/CsgoTeamIcon.vue'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'
import CsgoRoundKillDisplay from '@client/vue/utility/csgo/CsgoRoundKillDisplay.vue'

interface BombPlant {
    user: number | null
    site: CsgoBombSite | null
}

interface BombDefuse {
    user: number | null
}

interface RoundEvent {
    roundTimeMs: number
    kill?: CsgoRoundKill
    plant?: BombPlant
    defuse?: BombDefuse
    explode?: boolean
}

@Component({
    components: {
        CsgoTeamIcon,
        SteamAccountDisplay,
        CsgoRoundKillDisplay
    }
})
export default class CsgoEventRoundDisplay extends Vue {
    millisecondsToTimeString = millisecondsToTimeString
    bombSiteToString = bombSiteToString

    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    matchUserId!: number | null

    @Prop({required: true})
    currentRound!: number

    get startTime(): Date {
        return this.roundData?._r.tmRoundPlay || this.match._d.summary.matchStartTime
    }

    get allEvents(): RoundEvent[] {
        if (!this.roundData) {
            return []
        }

        let events: RoundEvent[] = []

        // Kills
        for (let k of this.roundData._r.kills) {
            events.push({
                roundTimeMs: k.tm.getTime() - this.startTime.getTime(),
                kill: k,
            })
        }

        // Bomb Plant
        if (this.roundData.hasBombPlant) {
            events.push({
                roundTimeMs: this.roundData._r.tmBombPlant!.getTime() - this.startTime.getTime(),
                plant: {
                    user: this.roundData._r.bombPlantUser,
                    site: this.roundData._r.bombPlantSite,
                }
            })
        }

        // Bomb Event (defuse or explode)
        if (this.roundData.hasBombDefuse) {
            events.push({
                roundTimeMs: this.roundData._r.tmBombEvent!.getTime() - this.startTime.getTime(),
                defuse: {
                    user: this.roundData._r.bombEventUser,
                }
            })
        } else if (this.roundData.hasBombExplode) {
            events.push({
                roundTimeMs: this.roundData._r.tmBombEvent!.getTime() - this.startTime.getTime(),
                explode: true,
            })
        }

        events.sort((a : RoundEvent, b : RoundEvent) => {
            return a.roundTimeMs - b.roundTimeMs
        })
        return events
    }

    get team(): CsgoTeam {
        if (this.matchUserId === null || !this.roundData) {
            return CsgoTeam.Spectate
        }
        return this.roundData.userTeam(this.matchUserId)
    }

    get roundData(): CsgoEventRoundWrapper | undefined {
        return this.match.round(this.currentRound)
    }

    get buyTime(): Date | null {
        return this.roundData!._r.tmRoundStart
    }

    get playTime(): Date | null {
        return this.roundData!._r.tmRoundPlay
    }

    get stopTime(): Date | null {
        return this.roundData!._r.tmRoundEnd
    }

    goToBuy() {
        this.$emit('go-to-event', this.buyTime!)
    }

    goToPlay() {
        this.$emit('go-to-event', this.playTime!)
    }

    goToStop() {
        this.$emit('go-to-event', this.stopTime!)
    }

    goToEvent(e: RoundEvent) {
        this.$emit('go-to-event', new Date(this.playTime!.getTime() + e.roundTimeMs))
    }

    eventStyling(e : RoundEvent) : any {
        // Left hand border (yellow) if this event directly concerns the selected player.
        // Background gradient (red vs green) for whether the event benefits the selected player.
        let style: any = {}

        if (this.matchUserId !== null && !!this.roundData) {
            let isSelf = false
            let isBenefit = false

            if (!!e.kill) {
                isSelf = e.kill.killer === this.matchUserId || e.kill.assister === this.matchUserId || e.kill.victim === this.matchUserId
                // Need to check for killer and assister to handle the case where we're only getting data from GSI and don't necessarily have a victim to team check.
                isBenefit = e.kill.victim !== this.matchUserId && (e.kill.victim !== null && this.team === this.roundData.userTeam(e.kill.victim)) || e.kill.killer === this.matchUserId || e.kill.assister === this.matchUserId
            } else if (!!e.plant) {
                isSelf = e.plant.user === this.matchUserId
                isBenefit = this.team === CsgoTeam.Terrorist
            } else if (!!e.defuse) {
                isSelf = e.defuse.user === this.matchUserId
                isBenefit = this.team === CsgoTeam.CT
            } else if (!!e.explode) {
                isSelf = false
                isBenefit = this.team === CsgoTeam.Terrorist
            }

            if (isSelf) {
                style['border-left'] = '5px solid #FFD700'
            } else {
                style['border-left'] = '5px solid transparent'
            }

            let gradientColor: Color = isBenefit ? getGenericWinColor() : getGenericLossColor()
            style['background'] = `linear-gradient(90deg, rgba(${gradientColor.r},${gradientColor.g},${gradientColor.b},0.0) 70%, rgba(${gradientColor.r},${gradientColor.g},${gradientColor.b},0.5) 100%)`
        }

        return style
    }

    userStyling(userId: number): any {
        if (!this.roundData) {
            return {}
        }
        let color: Color = getGenericLossColor()
        let refTeam: CsgoTeam = this.roundData.userTeam(!!this.matchUserId ? this.matchUserId : 0)
        let inputTeam: CsgoTeam = this.roundData.userTeam(userId)
        if (userId === this.matchUserId) {
            color = getGenericFirstPlaceColor()
        } else if (refTeam === inputTeam) {
            color = getGenericWinColor()
        } else {
            color = getGenericLossColor()
        }

        return {
            'border': `2px solid ${colorToCssString(color)}`
        }
    }
}

</script>

<style scoped>

.event-list {
    overflow-y: auto;
    height: calc(100% - 72px);
}

.event-time {
    flex: 0 0 auto !important;
}

</style>