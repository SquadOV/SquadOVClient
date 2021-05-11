<template>
    <v-expansion-panels>
        <v-expansion-panel
            v-for="(data, i) in h2hData"
            :key="i"
        >   
            <v-expansion-panel-header>
                <span><span class="font-weight-bold">{{ data.kills }}</span> Kills</span>
                <span><span class="font-weight-bold">{{ data.deaths }}</span> Deaths</span>
                <span><span class="font-weight-bold">{{ data.assists }}</span> Assists</span>
                <span class="text-overline">Versus</span>
                <span v-if="!!data.against">{{ data.against.name }}</span>
                <steam-account-display
                    :account="data.against"
                    :width="48"
                    :height="48"
                >
                </steam-account-display>
            </v-expansion-panel-header>

            <v-expansion-panel-content>
                <v-list>
                    <v-list-item
                        :class="`${ kill.isKill ? 'kill-highlight' : kill.isAssist ? 'assist-highlight' : 'death-highlight'}`"
                        v-for="(kill, index) in data.instances"
                        :key="index"
                        two-line
                    >
                        <v-list-item-content class="round-id flex-grow-0">
                            <v-list-item-title>
                                Round {{ kill.roundNum + 1 }}
                            </v-list-item-title>

                            <v-list-item-subtitle>
                                {{ millisecondsToTimeString(kill.roundTimeMs) }}
                            </v-list-item-subtitle>
                        </v-list-item-content>

                        <v-list-item-content>
                            <div class="d-flex align-center justify-space-around">
                                <steam-account-display
                                    v-if="!!kill.source"
                                    :account="kill.source"
                                    :height="48"
                                >
                                </steam-account-display>

                                <csgo-weapon-icon
                                    :weapon="kill.weapon"
                                    :height="48"
                                >
                                </csgo-weapon-icon>

                                <steam-account-display
                                    v-if="!!data.against"
                                    :account="data.against"
                                    :height="48"
                                >
                                </steam-account-display>
                            </div>
                        </v-list-item-content>

                        <v-list-item-action class="ml-0">
                            <v-btn
                                outlined
                                fab
                                small
                                color="white"
                                @click="goToTime(kill.tm)"
                            >
                                <v-icon>mdi-play</v-icon>
                            </v-btn>
                        </v-list-item-action>
                    </v-list-item>
                </v-list>
            </v-expansion-panel-content>
        </v-expansion-panel>
    </v-expansion-panels>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoFullMatchDataWrapper } from '@client/js/csgo/match'
import { SteamAccount } from '@client/js/steam/account'
import { millisecondsToTimeString } from '@client/js/time'
import { CsgoWeapon } from '@client/js/csgo/events'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'
import CsgoWeaponIcon from '@client/vue/utility/csgo/CsgoWeaponIcon.vue'

interface CsgoH2hData {
    againstUserId: number
    against: SteamAccount | undefined
    kills: number
    deaths: number
    assists: number
    instances: {
        source: SteamAccount | undefined
        roundNum: number
        roundTimeMs: number
        tm: Date
        isKill: boolean
        isAssist: boolean
        weapon: CsgoWeapon
    }[]
}

@Component({
    components: {
        SteamAccountDisplay,
        CsgoWeaponIcon,
    }
})
export default class CsgoHeadToHeadDisplay extends Vue {
    millisecondsToTimeString = millisecondsToTimeString

    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    matchUserId!: number | null

    goToTime(d: Date) {
        this.$emit('go-to-event', d)
    }

    get h2hData(): CsgoH2hData[] {
        let data: Map<number, CsgoH2hData> = new Map()

        for (let r of this.match.rounds) {
            for (let k of r._r.kills) {
                let againstUserId: number | null = null
                let sourceUserId: number | null = null
                let isKill: boolean = false
                let isAssist: boolean = false
                let isDeath: boolean = false

                if (k.killer === this.matchUserId) {
                    isKill = true
                    againstUserId = k.victim
                    sourceUserId = k.killer
                } else if (k.assister === this.matchUserId) {
                    isAssist = true
                    againstUserId = k.victim
                    sourceUserId = k.killer
                } else if (k.victim === this.matchUserId) {
                    isDeath = true
                    againstUserId = k.killer
                    sourceUserId = k.victim
                }

                if (againstUserId === null || sourceUserId === null || !k.weapon) {
                    continue
                }

                let againstAccount = this.match.steamAccount(this.match.userIdToSteamId(againstUserId))
                let sourceAccount = this.match.steamAccount(this.match.userIdToSteamId(sourceUserId))

                if (!data.has(againstUserId)) {
                    data.set(againstUserId, {
                        againstUserId: againstUserId,
                        against: againstAccount,
                        kills: 0,
                        deaths: 0,
                        assists: 0,
                        instances: [],
                    })
                }

                let h2h = data.get(againstUserId)!
                if (isKill) {
                    h2h.kills += 1
                } else if (isAssist) {
                    h2h.assists += 1
                } else if (isDeath) {
                    h2h.deaths += 1
                } else {
                    continue
                }

                h2h.instances.push({
                    source: sourceAccount,
                    roundNum: r.roundNum,
                    roundTimeMs: k.tm.getTime() - (r._r.tmRoundPlay || this.match._d.summary.matchStartTime).getTime(),
                    tm: k.tm,
                    isKill: isKill,
                    isAssist: isAssist,
                    weapon: k.weapon,
                })
            }
        }

        return Array.from(data.values()).sort((a: CsgoH2hData, b: CsgoH2hData) => {
            return b.kills - a.kills
        })
    }
}

</script>

<style scoped>

.round-id {
    flex-basis: auto;
}

.kill-highlight {
    border-left: 3px solid #4CAF50;
}

.assist-highlight {
    border-left: 3px solid #FFD700;
}

.death-highlight {
    border-left: 3px solid #FF5252;
}

</style>