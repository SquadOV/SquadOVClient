<template>
    <v-expansion-panels>
        <v-expansion-panel
            v-for="(against, i) in uniqueAgainst"
            :key="i"
        >   
            <v-expansion-panel-header>
                <span><span class="font-weight-bold">{{ numKillsAgainst(against) }}</span> Kills</span>
                <span><span class="font-weight-bold">{{ numDeathsAgainst(against) }}</span> Deaths</span>
                <span class="text-overline">Versus</span>
                <span>{{ match.getPlayerAgentName(against) }}</span>
                <valorant-agent-icon
                    :agent="match.getPlayerAgentId(against)"
                    :patch="match._details.patchId"
                    :width-height="40"
                >
                </valorant-agent-icon>    
            </v-expansion-panel-header>

            <v-expansion-panel-content>
                <v-list>
                    <v-list-item
                         :class="`${ (kill.victim._p.puuid == against) ? 'kill-highlight' : 'death-highlight'}`"
                        v-for="(kill, index) in chronologicalKillsDeathsAgainst(against)"
                        :key="index"
                        two-line
                    >
                        <v-list-item-content class="round-id flex-grow-0">
                            <v-list-item-title>
                                Round {{ kill._k.roundNum + 1 }}
                            </v-list-item-title>

                            <v-list-item-subtitle>
                                {{ formatRoundTime(kill._k.roundTime) }}
                            </v-list-item-subtitle>
                        </v-list-item-content>

                        <v-list-item-content>
                            <div class="d-flex align-center justify-space-around">
                                <valorant-agent-icon
                                    :agent="kill.killer._p.agentId"
                                    :patch="match._details.patchId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <valorant-weapon-ability-icon
                                    :agent="kill.killer._p.agentId"
                                    :patch="match._details.patchId"
                                    :equip-type="kill._k.damageType"
                                    :equip-id="kill._k.damageItem"
                                    :max-height="40"
                                    :max-width="150"
                                >
                                </valorant-weapon-ability-icon>

                                <valorant-agent-icon
                                    :agent="kill.victim._p.agentId"
                                    :patch="match._details.patchId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>
                            </div>
                        </v-list-item-content>

                        <v-list-item-action class="ml-0" v-if="canGoToKill(kill)">
                            <v-btn
                                outlined
                                fab
                                small
                                color="white"
                                @click="goToKill(kill)"
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
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchKillWrapper,
} from '@client/js/valorant/valorant_matches_parsed'
import { formatRoundTime } from '@client/js/valorant/valorant_utility'

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantWeaponAbilityIcon from '@client/vue/utility/valorant/ValorantWeaponAbilityIcon.vue'

// This constant is shared with ValorantRoundEvents.
const offsetMs = 1500

@Component({
    components: {
        ValorantAgentIcon,
        ValorantWeaponAbilityIcon
    }
})
export default class ValorantHeadToHeadDisplay extends Vue {
    formatRoundTime : any = formatRoundTime

    @Prop({type:Array, required: true})
    kills! : ValorantMatchKillWrapper[]

    @Prop({type:Array, required: true})
    deaths! : ValorantMatchKillWrapper[]

    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    canGoToKill(kill : ValorantMatchKillWrapper) : boolean {
        return !!this.match._details.ovStartTime &&
            !!this.match.getRound(kill._k.roundNum)?._r.startPlayTime
    }

    goToKill(kill : ValorantMatchKillWrapper) {
        let rnd = this.match.getRound(kill._k.roundNum)!
        let start : Date = rnd._r.startPlayTime!
        this.$emit('go-to-event', new Date(start.getTime() + kill._k.roundTime - offsetMs))
    }

    killsAgainst(puuid : string) : ValorantMatchKillWrapper[] {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return ele.victim._p.puuid == puuid
        })
    }

    deathsAgainst(puuid : string) : ValorantMatchKillWrapper[] {
        return this.deaths.filter((ele : ValorantMatchKillWrapper) => {
            return ele.killer._p.puuid == puuid
        })
    }

    chronologicalKillsDeathsAgainst(puuid : string) : ValorantMatchKillWrapper[] {
        let all = [...this.killsAgainst(puuid), ...this.deathsAgainst(puuid)]
        return all.sort((a : ValorantMatchKillWrapper, b: ValorantMatchKillWrapper) => {
            return (a._k.roundNum - b._k.roundNum) || (a._k.roundTime - b._k.roundTime)
        })
    }

    numKillsAgainst(puuid : string) : number {
        return this.killsAgainst(puuid).length
    }

    numDeathsAgainst(puuid : string) : number {
        return this.deathsAgainst(puuid).length
    }

    // Unique players we kills/died from.
    get uniqueAgainst() : string[] {
        let against = new Set<string>()

        for (let k of this.kills) {
            if (k.isSelfTeamKill) {
                continue
            }
            against.add(k.victim._p.puuid)
        }

        for (let d of this.deaths) {
            if (d.isSelfTeamKill) {
                continue
            }
            against.add(d.killer._p.puuid)
        }

        return Array.from(against).sort((a : string, b : string) => {
            return this.numKillsAgainst(b) - this.numKillsAgainst(a)
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

.death-highlight {
    border-left: 3px solid #FF5252;
}

</style>