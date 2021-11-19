<template>
    <v-container fluid>
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
                <valorant-filter-ui
                    v-model="filters"
                    class="mb-2"
                    :saved-filter-loc="DataStorageLocation.Valorant"
                >
                </valorant-filter-ui>
                
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
                                :user-id="userId"
                                :account="puuid"
                            >
                            </valorant-match-scroller>

                            <div class="d-flex justify-center align-center text-h5" v-else>
                                <span>
                                No games found. Open Valorant and play a game to start recording.
                                If you just added the account and Valorant is running, you will need to <span class="font-weight-bold">restart</span> Valorant for this to work.
                                </span>
                            </div>
                        </div>
                    </template>
                </loading-container>
            </v-col>
        </v-row>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import { ValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'
import { ValorantMatchFilters, createEmptyValorantMatchFilters } from '@client/js/valorant/filters'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantPlayerCard from '@client/vue/utility/valorant/ValorantPlayerCard.vue'
import ValorantMatchScroller from '@client/vue/utility/valorant/ValorantMatchScroller.vue'
import ValorantFilterUi from '@client/vue/utility/valorant/ValorantFilterUi.vue'
import { DataStorageLocation } from '@client/js/system/data_storage'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        ValorantPlayerCard,
        ValorantMatchScroller,
        ValorantFilterUi
    }
})
export default class ValorantGameLog extends Vue {
    DataStorageLocation = DataStorageLocation

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    puuid!: string | undefined

    account : RiotAccountData | null = null
    playerStats : ValorantPlayerStatsSummary | null = null

    allMatches : ValorantPlayerMatchSummary[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null
    filters: ValorantMatchFilters = createEmptyValorantMatchFilters()

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('puuid')
    refreshAccount() {
        if (!this.puuid) {
            this.account = null;
            return null;
        }

        apiClient.getValorantAccount(this.userId, this.puuid).then((resp : ApiData<RiotAccountData>) => {
            this.account = resp.data
        }).catch((err : any) => {
            console.error('Failed to get valorant account: ', err);
        })
    }

    get accountPuuid() : string | null {
        if (!this.account) {
            return null
        }
        return this.account.puuid
    }

    @Watch('filters', {deep: true})
    onFilterChange() {
        if (!this.accountPuuid) {
            return
        }

        this.loadMoreMatches(this.accountPuuid, null)
    }

    @Watch('accountPuuid')
    loadMoreMatches(newPuuid : string | null, oldPuuid : string | null) {
        if (!this.accountPuuid) {
            return
        }

        if (newPuuid !== oldPuuid) {
            this.allMatches = null
            this.nextLink = null
            this.lastIndex = 0
        }

        if (!!this.allMatches && !this.nextLink) {
            return
        }

        apiClient.listValorantMatchesForPlayer({
            next: this.nextLink,
            userId: this.userId,
            puuid: this.accountPuuid!,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filters: this.filters,
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
            console.error('Failed to list valorant matches: ' + err);
        })
    }

    @Watch('accountPuuid')
    refreshPlayerStats() {
        if (!this.accountPuuid) {
            this.playerStats = null
            return
        }

        apiClient.getValorantPlayerStats(this.userId, this.accountPuuid).then((resp : ApiData<ValorantPlayerStatsSummary>) => {
            this.playerStats = resp.data
        }).catch((err : any) => {
            console.error('Failed to get player stats: ' + err);
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