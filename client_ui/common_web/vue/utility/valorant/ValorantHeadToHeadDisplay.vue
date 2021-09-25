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
                                Round {{ kill.roundNum + 1 }}
                            </v-list-item-title>

                            <v-list-item-subtitle>
                                {{ formatRoundTime(kill._k.timeSinceRoundStartMillis) }}
                            </v-list-item-subtitle>
                        </v-list-item-content>

                        <v-list-item-content>
                            <div class="d-flex align-center justify-space-around">
                                <valorant-agent-icon
                                    v-if="!!kill.killer"
                                    :agent="kill.killer._p.characterId"
                                    :width-height="40"
                                    circular
                                >
                                </valorant-agent-icon>

                                <valorant-weapon-ability-icon
                                    :agent="kill.killer._p.characterId"
                                    :equip-type="kill._k.finishingDamage.damageType"
                                    :equip-id="kill._k.finishingDamage.damageItem"
                                    :max-height="40"
                                    :max-width="150"
                                    reverse-weapon
                                >
                                </valorant-weapon-ability-icon>

                                <valorant-agent-icon
                                    :agent="kill.victim._p.characterId"
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

import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantMatchPlayerMatchMetadata } from '@client/js/valorant/valorant_matches'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchKillWrapper,
} from '@client/js/valorant/valorant_matches_parsed'
import { formatRoundTime } from '@client/js/valorant/valorant_utility'

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantWeaponAbilityIcon from '@client/vue/utility/valorant/ValorantWeaponAbilityIcon.vue'
import CommonComponent from '@client/vue/CommonComponent'

// This constant is shared with ValorantRoundEvents.
const offsetMs = 1500

@Component({
    components: {
        ValorantAgentIcon,
        ValorantWeaponAbilityIcon
    }
})
export default class ValorantHeadToHeadDisplay extends mixins(CommonComponent) {
    formatRoundTime : any = formatRoundTime

    @Prop({type:Array, required: true})
    kills! : ValorantMatchKillWrapper[]

    @Prop({type:Array, required: true})
    deaths! : ValorantMatchKillWrapper[]

    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop({default: null})
    metadata!: ValorantMatchPlayerMatchMetadata | null

    @Prop({type: Boolean, default: false})
    forceDisableGoToEvent! : boolean

    canGoToKill(kill : ValorantMatchKillWrapper) : boolean {
        return !this.forceDisableGoToEvent &&
            !!this.metadata?.rounds[kill.roundNum]?.roundTime
    }

    goToKill(kill : ValorantMatchKillWrapper) {
        let roundStart = this.metadata?.rounds[kill.roundNum]?.roundTime!
        let tm = roundStart.getTime() + kill._k.timeSinceRoundStartMillis - offsetMs
        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.GoToEvent,
            'Kill',
            tm
        )
        this.$emit('go-to-event', new Date(tm))
    }

    killsAgainst(puuid : string) : ValorantMatchKillWrapper[] {
        return this.kills.filter((ele : ValorantMatchKillWrapper) => {
            return ele.victim._p.puuid == puuid
        })
    }

    deathsAgainst(puuid : string) : ValorantMatchKillWrapper[] {
        return this.deaths.filter((ele : ValorantMatchKillWrapper) => {
            return ele.killer?._p.puuid == puuid
        })
    }

    chronologicalKillsDeathsAgainst(puuid : string) : ValorantMatchKillWrapper[] {
        let all = [...this.killsAgainst(puuid), ...this.deathsAgainst(puuid)]
        return all.sort((a : ValorantMatchKillWrapper, b: ValorantMatchKillWrapper) => {
            return (a._k.timeSinceGameStartMillis - b._k.timeSinceGameStartMillis)
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

            if (!!d.killer) {
                against.add(d.killer._p.puuid)
            }
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
    border-left: 3px solid var(--color-friendly);
}

.death-highlight {
    border-left: 3px solid var(--color-enemy);
}

</style>