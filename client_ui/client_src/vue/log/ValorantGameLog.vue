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
                <loading-container :is-loading="!playerStats">
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
                                v-if="allMatches.length > 0"
                                :matches="allMatches"
                                :can-load-more="hasNext"
                                @load-more="loadMoreMatches"
                            >
                            </valorant-match-scroller>

                            <v-row justify="center" align="center" v-else>
                                <span class="text-h5">
                                    No games found. Open Valorant and play a game to sync your matches.
                                    If you just added the account and Valorant is running, you will need to <span class="font-weight-bold">restart</span> Valorant for this to work.
                                </span>
                            </v-row>
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
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { ValorantAccountData } from '@client/js/valorant/valorant_account'
import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import { ValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantPlayerCard from '@client/vue/utility/valorant/ValorantPlayerCard.vue'
import ValorantMatchScroller from '@client/vue/utility/valorant/ValorantMatchScroller.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        ValorantPlayerCard,
        ValorantMatchScroller
    }
})
export default class ValorantGameLog extends Vue {
    account : ValorantAccountData | null = null
    playerStats : ValorantPlayerStatsSummary | null = null

    allMatches : ValorantPlayerMatchSummary[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get routeAccountPuuid() : string | null {
        if (!this.$route.params.account) {
            return null;
        }
        return this.$route.params.account
    }

    get hasNext() : boolean {
        return !!this.nextLink
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
    loadMoreMatches(newPuuid : string | null, oldPuuid : string | null) {
        if (!this.accountPuuid) {
            return
        }

        if (newPuuid !== oldPuuid) {
            this.allMatches = null
            this.nextLink = null
        }

        if (!!this.allMatches && !this.nextLink) {
            return
        }

        apiClient.listValorantMatchesForPlayer({
            next: this.nextLink,
            userId: this.$store.state.currentUser.id,
            puuid: this.accountPuuid!,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
        }).then((resp : ApiData<HalResponse<ValorantPlayerMatchSummary[]>>) => {
            if (!this.allMatches) {
                this.allMatches = resp.data.data
            } else {
                this.allMatches.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
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

        apiClient.getValorantPlayerStats(this.$store.state.currentUser.id, this.accountPuuid).then((resp : ApiData<ValorantPlayerStatsSummary>) => {
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