<template>
    <v-card>
        <v-card-title class="justify-center">
            <valorant-rank-tier
                :value="summary.rank"
                class="mr-4"
            >
            </valorant-rank-tier>
            <div>
                <div>{{ account.username }}#{{ account.tag }}</div>

                <div class="text-overline">
                    <win-loss-stat
                        :wins="summary.wins"
                        :games="summary.games"
                    >
                    </win-loss-stat>
                </div>
            </div>
        </v-card-title>

        <v-card-text>
            <v-row>
                <v-col cols="6">
                    <generic-stat
                        name="KDA"
                        desc="(Kills + Deaths) / Assists"
                        :value="kda"
                    >
                    </generic-stat>

                    <generic-stat
                        name="HS %"
                        desc="Headshot Percentage"
                        :value="hsp"
                        percentage
                    >
                    </generic-stat>
                </v-col>

                <v-col cols="6">
                    <generic-stat
                        name="DPR"
                        desc="Average Damage Per Round"
                        :value="dpr"
                    >
                    </generic-stat>

                    <generic-stat
                        name="CSPR"
                        desc="Average Combat Score Per Round"
                        :value="cspr"
                    >
                    </generic-stat>
                </v-col>
            </v-row>
        </v-card-text>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantPlayerStatsSummary, kda, hsp, dpr, cspr } from '@client/js/valorant/valorant_player_stats'
import { RiotAccountData } from '@client/js/valorant/valorant_account'

import ValorantRankTier from '@client/vue/utility/valorant/ValorantRankTier.vue'
import WinLossStat from '@client/vue/utility/WinLossStat.vue'
import GenericStat from '@client/vue/utility/GenericStat.vue'

@Component({
    components: {
        WinLossStat,
        ValorantRankTier,
        GenericStat
    }
})
export default class ValorantPlayerCard extends Vue {
    @Prop({required: true})
    summary! : ValorantPlayerStatsSummary

    @Prop({required: true})
    account! : RiotAccountData

    get kda() : number {
        return kda(this.summary.kills, this.summary.assists, this.summary.deaths)
    }

    get hsp() : number {
        return hsp(this.summary.headshots, this.summary.bodyshots, this.summary.legshots)
    }

    get dpr() : number {
        return dpr(this.summary.totalDamage, this.summary.rounds)
    }

    get cspr() : number {
        return cspr(this.summary.totalCombatScore, this.summary.rounds)
    }
}

</script>

<style scoped>

</style>