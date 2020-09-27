<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <v-row>
                    <valorant-match-header-summary
                        :match="matchWrapper"
                        :current-player="puuid"
                    >
                    </valorant-match-header-summary>
                </v-row>

                <v-row>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :video-filename="vodFilename"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                        >
                        </video-player>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <valorant-round-events
                            :match="matchWrapper"
                            :round="currentRound"
                            :force-disable-go-to-event="!hasMatchTiming"
                            :style="roundEventsStyle"
                            :current-player="currentPlayer"
                            @go-to-event="goToVodTime"
                        >
                        </valorant-round-events>
                    </v-col>
                </v-row>

                <v-row>
                </v-row>

                <v-row>
                </v-row>

                <v-row>

                </v-row>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { ValorantMatchDetails } from '@client/js/valorant/valorant_matches'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchRoundWrapper,
    ValorantMatchPlayerWrapper
} from '@client/js/valorant/valorant_matches_parsed'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantMatchHeaderSummary from '@client/vue/utility/valorant/ValorantMatchHeaderSummary.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import ValorantRoundEvents from '@client/vue/utility/valorant/ValorantRoundEvents.vue'

@Component({
    components: {
        LoadingContainer,
        ValorantMatchHeaderSummary,
        VideoPlayer,
        ValorantRoundEvents
    }
})
export default class ValorantMatch extends Vue {
    @Prop()
    puuid! : string | null

    @Prop({required: true})
    matchId! : string

    $refs!: {
        player: VideoPlayer
    }
    currentPlayerHeight : number = 0

    currentMatch : ValorantMatchDetails | null = null
    currentPlayer : ValorantMatchPlayerWrapper | null = null
    currentRoundNum : number = 0

    theaterMode: boolean = false

    get matchWrapper() : ValorantMatchDetailsWrapper | null {
        if (!this.currentMatch) {
            return null
        }
        return new ValorantMatchDetailsWrapper(this.currentMatch)
    }

    get currentRound() : ValorantMatchRoundWrapper | null {
        if (!this.currentMatch) {
            return null
        }

        return this.matchWrapper!.getRound(this.currentRoundNum)
    }

    get vodFilename() : string | null {
        if (!this.currentMatch) {
            return ''
        }
        return this.currentMatch.vodPath
    }

    get hasMatchTiming() : boolean {
        return !!this.currentMatch!.ovStartTime
    }

    get roundEventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight}px`,
        }
    }

    goToVodTime(tm : Date) {
        // Need to estimate what the video time is based on the given
        // Date and the Date we have for when the match started.
        let matchStartTime : Date | null = this.currentMatch!.ovStartTime
        if (!matchStartTime) {
            return
        }

        let diffMs = tm.getTime() - matchStartTime.getTime()
        this.$refs.player.goToTimeMs(diffMs)
    }

    @Watch('puuid')
    @Watch('currentMatch')
    refreshCurrentPlayer() {
        if (!this.currentMatch || !this.puuid) {
            this.currentPlayer = null
            return
        }
        this.currentPlayer = this.matchWrapper!.getPlayer(this.puuid)
    }

    @Watch('matchId')
    refreshMatch() {
        apiClient.getValorantMatchDetails(this.matchId).then((resp : ApiData<ValorantMatchDetails>) => {
            this.currentMatch = resp.data
        }).catch((err : any) => {
            console.log('Failed to get valorant match details: ' + err)
        })
    }

    mounted() {
        this.refreshMatch()
    }
}
</script>

<style scoped>

#match-vod {
    min-height: 500px;
    width: 100%;
}

</style>