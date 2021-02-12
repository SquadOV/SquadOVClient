<template>
    <v-container fluid id="tftGameLog">
        <v-row>
            <v-col cols="12">
                <!-- 
                    Game log
                -->
                <loading-container :is-loading="!allMatches">
                    <template v-slot:default="{ loading }">
                        <div v-if="!loading">
                            <template v-if="allMatches.length > 0">
                                <tft-match-summary
                                    class="mb-2"
                                    v-for="(match, index) in allMatches"
                                    :key="index"
                                    :match="match"
                                    :puuid="puuid"
                                    :user-id="userId"
                                >
                                </tft-match-summary>

                                <v-btn
                                    v-if="hasNext"
                                    color="primary"
                                    block
                                    @click="loadMoreMatches(puuid, puuid)"  
                                >
                                    Load More
                                </v-btn>
                            </template>

                            <div class="d-flex justify-center align-center text-h5" v-else>
                                <span>
                                No games found. Open TFT and play a game to sync your matches.
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
import { TftPlayerMatchSummary } from '@client/js/tft/matches'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import TftMatchSummary from '@client/vue/utility/tft/TftMatchSummary.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        TftMatchSummary
    }
})
export default class TftGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    puuid!: string | undefined

    allMatches : TftPlayerMatchSummary[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    refreshData() {
        this.allMatches = null
        this.nextLink = null
        this.lastIndex = 0
        this.loadMoreMatches(this.puuid, this.puuid)
    }

    @Watch('puuid')
    loadMoreMatches(newPuuid : string | null | undefined, oldPuuid : string | null | undefined) {
        if (!this.puuid) {
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

        apiClient.listTftMatchesForPlayer({
            next: this.nextLink,
            userId: this.userId,
            puuid: this.puuid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
        }).then((resp : ApiData<HalResponse<TftPlayerMatchSummary[]>>) => {
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
            console.log('Failed to list TFT matches: ' + err);
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

#tftGameLog {
    overflow-y: auto;
    overflow-x: hidden;
}

</style>