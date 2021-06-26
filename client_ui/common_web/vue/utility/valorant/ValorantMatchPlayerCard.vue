<template>
    <div>
        <div class="pa-4 d-flex justify-space-around align-center">
            <valorant-rank-tier :value="currentPlayer._p.competitiveTier" :max-width-height="50">
            </valorant-rank-tier>

            <span class="text-h5">{{ agentName(currentPlayer._p.characterId) }}</span>

            <valorant-agent-icon
                :agent="currentPlayer._p.characterId"
                :width-height="50"
                circular
            >
            </valorant-agent-icon>
        </div>
        <v-divider></v-divider>
        <v-tabs grow color="primary" v-model="tab">
            <v-tab>
                Summary
            </v-tab>

            <v-tab-item>
                <v-container fluid>
                    <v-row no-gutters>
                        <v-col cols="6">
                            <generic-stat
                                name="KDA"
                                desc="(Kills + Deaths) / Assists"
                                :value="currentPlayer.kda"
                            >
                            </generic-stat>

                            <generic-stat
                                name="HS %"
                                desc="Headshot Percentage"
                                :value="currentPlayer.hsp"
                                percentage
                            >
                            </generic-stat>

                            <generic-stat
                                name="FB"
                                desc="First Bloods"
                                :value="match.firstBloods(currentPlayer._p.puuid)"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="DK"
                                desc="Double Kills"
                                :value="currentPlayer.doubleKills"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="QK"
                                desc="Quadra Kills"
                                :value="currentPlayer.quadraKills"
                                force-integer
                            >
                            </generic-stat>
                        </v-col>

                        <v-col cols="6">
                            <generic-stat
                                name="DPR"
                                desc="Average Damage Per Round"
                                :value="currentPlayer.dpr"
                            >
                            </generic-stat>

                            <generic-stat
                                name="CSPR"
                                desc="Average Combat Score Per Round"
                                :value="currentPlayer.cspr"
                            >
                            </generic-stat>

                            <generic-stat
                                name="FTD"
                                desc="First To Die"
                                :value="match.firstToDie(currentPlayer._p.puuid)"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="TK"
                                desc="Triple Kills"
                                :value="currentPlayer.tripleKills"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="PK"
                                desc="Penta Kills"
                                :value="currentPlayer.pentaKills"
                                force-integer
                            >
                            </generic-stat>
                        </v-col>
                    </v-row>

                    <v-row no-gutters>
                        <valorant-hit-tracker
                            :headshots="currentPlayer.headshots"
                            :bodyshots="currentPlayer.bodyshots"
                            :legshots="currentPlayer.legshots"
                        >
                        </valorant-hit-tracker>
                    </v-row>
                </v-container>
            </v-tab-item>

            <v-tab>
                Weapons
            </v-tab>

            <v-tab-item>
                <valorant-weapon-ability-kill-stats
                    :kills="currentPlayer.kills"
                    :agent="currentPlayer._p.characterId"
                >
                </valorant-weapon-ability-kill-stats>
            </v-tab-item>

            <v-tab>
                H2H
            </v-tab>

            <v-tab-item>
                <valorant-head-to-head-display
                    :kills="currentPlayer.kills"
                    :deaths="currentPlayer.deaths"
                    :match="match"
                    :metadata="metadata"
                    :force-disable-go-to-event="forceDisableGoToEvent"
                    @go-to-event="$emit('go-to-event', arguments[0])"
                >
                </valorant-head-to-head-display>
            </v-tab-item>
        </v-tabs>
    </div>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { ValorantMatchPlayerMatchMetadata } from '@client/js/valorant/valorant_matches'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchPlayerWrapper,
} from '@client/js/valorant/valorant_matches_parsed'
import { getValorantContent } from '@client/js/valorant/valorant_content'

import ValorantRankTier from '@client/vue/utility/valorant/ValorantRankTier.vue'
import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantHitTracker from '@client/vue/utility/valorant/ValorantHitTracker.vue'
import ValorantWeaponAbilityKillStats from '@client/vue/utility/valorant/ValorantWeaponAbilityKillStats.vue'
import ValorantHeadToHeadDisplay from '@client/vue/utility/valorant/ValorantHeadToHeadDisplay.vue'
import GenericStat from '@client/vue/utility/GenericStat.vue'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        ValorantRankTier,
        ValorantAgentIcon,
        ValorantHitTracker,
        ValorantWeaponAbilityKillStats,
        ValorantHeadToHeadDisplay,
        GenericStat
    }
})
export default class ValorantMatchPlayerCard extends mixins(CommonComponent) {
    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop()
    currentPlayer! : ValorantMatchPlayerWrapper

    @Prop({default: null})
    metadata!: ValorantMatchPlayerMatchMetadata | null

    @Prop({type: Boolean, default: false})
    forceDisableGoToEvent! : boolean

    tab: number = 0

    get tabLabel(): string {
        switch (this.tab) {
            case 0:
                return 'Summary'
            case 1:
                return 'Weapons'
            case 2:
                return 'H2H'
        }
        return ''
    }

    @Watch('tab')
    onChangeTab() {
        this.sendAnalyticsEvent(
            this.AnalyticsCategory.MatchInfo,
            this.AnalyticsAction.NavigateMatchInfo,
            this.tabLabel,
            this.tab
        )
    }

    agentName(id : string) : string {
        let cnt = getValorantContent(null)
        return cnt.agentIdToName(id)
    }
}

</script>