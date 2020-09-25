<template>
    <div id="valorantGameLog">
        <v-row>
            <!-- Filtering options -->
        </v-row>

        <v-row>
            <v-col cols="4">
                <!-- 
                    Stats highlight.
                -->
                <loading-container :is-loading="!playerStats || !account">
                    <template v-slot:default="{ loading }">
                        <valorant-player-card 
                            v-if="!loading"
                            :account="account"
                            :summary="playerStats"
                        >
                        </valorant-player-card>
                    </template>
                </loading-container>
            </v-col>

            <v-col cols="8">
                <!-- 
                    Game log
                -->
                <loading-container :is-loading="!allMatches">
                    <template v-slot:default="{ loading }">
                        <div v-if="!loading">
                            <valorant-match-scroller
                                :matches="allMatches"
                            >
                            </valorant-match-scroller>
                        </div>
                    </template>
                </loading-container>
            </v-col>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { ValorantAccountData } from '@client/js/valorant/valorant_account'
import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import { ValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantPlayerCard from '@client/vue/utility/valorant/ValorantPlayerCard.vue'
import ValorantMatchScroller from '@client/vue/utility/valorant/ValorantMatchScroller.vue'

@Component({
    components: {
        LoadingContainer,
        ValorantPlayerCard,
        ValorantMatchScroller
    }
})
export default class ValorantGameLog extends Vue {
    account : ValorantAccountData | null = null
    allMatches : ValorantPlayerMatchSummary[] | null = []
    playerStats : ValorantPlayerStatsSummary | null = null

    get routeAccountPuuid() : string | null {
        if (!this.$route.params.account) {
            return null;
        }
        return this.$route.params.account
    }

    @Watch('routeAccountPuuid')
    refreshAccount() {
        if (!this.routeAccountPuuid) {
            this.account = null;
            return null;
        }

        apiClient.getValorantAccount(this.routeAccountPuuid).then((resp : ApiData<ValorantAccountData>) => {
            this.account = resp.data
        }).catch((err : any) => {
            console.log('Failed to get valorant account: ' + err);
        })       
    }

    get accountPuuid() : string | null {
        if (!this.account) {
            return null
        }
        return this.account.puuid
    }

    @Watch('accountPuuid')
    refreshMatches() {
        if (!this.accountPuuid) {
            this.allMatches = null
            return
        }

        apiClient.listValorantMatchesForPlayer(this.accountPuuid).then((resp : ApiData<ValorantPlayerMatchSummary[]>) => {
            this.allMatches = resp.data
        }).catch((err : any) => {
            console.log('Failed to list valorant matches: ' + err);
        })
    }

    @Watch('accountPuuid')
    refreshPlayerStats() {
        if (!this.accountPuuid) {
            this.playerStats = null
            return
        }

        apiClient.getValorantPlayerStats(this.accountPuuid).then((resp : ApiData<ValorantPlayerStatsSummary>) => {
            this.playerStats = resp.data
        }).catch((err : any) => {
            console.log('Failed to get player stats: ' + err);
        })
    }

    mounted() {
        this.refreshAccount()
    }
}

</script>

<style scoped>

#valorantGameLog {
    overflow-y: auto;
    overflow-x: hidden;
}

</style>