<template>
    <loading-container :is-loading="!match">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <!-- Header summary -->
                <div class="d-flex align-center">
                    <csgo-player-match-summary-display
                        class="flex-grow-1 mb-4"
                        :match="match.summary"
                        :user-id="userId"
                        disable-click
                    >
                    </csgo-player-match-summary-display>

                    <match-favorite-button
                        :match-uuid="matchUuid"
                    >
                    </match-favorite-button>

                    <match-share-button
                        :match-uuid="matchUuid"
                        :game="SquadOvGames.Csgo"
                        :user-id="userId"
                    >
                    </match-share-button>
                </div>

                <!-- VOD + Event List -->
                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :vod="vod"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                            :current-time.sync="vodTime"
                            :ready.sync="vodReady"
                            :enable-draw="enableDraw"
                        >
                        </video-player>

                        <csgo-vod-picker
                            :vod.sync="vod"
                            :match-uuid="matchUuid"
                            :match="matchData"
                            :current-steam-id="currentSteamId"
                            :timestamp="vodTime"
                            disable-favorite
                            :enable-draw.sync="enableDraw"
                        >
                        </csgo-vod-picker>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                    </v-col>
                </v-row>

                <!-- Round Timeline -->
                <v-row class="my-4">
                    <csgo-round-timeline
                        :match="matchData"
                        :match-user-id="selectedMatchUserId"
                        :current-round.sync="currentRound"
                    >
                    </csgo-round-timeline>
                </v-row>

                <!-- Scoreboard and player summary -->
                <v-row class="my-4">

                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'
import { CsgoFullMatchData, CsgoFullMatchDataWrapper } from '@client/js/csgo/match'
import { VodAssociation } from '@client/js/squadov/vod'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import CsgoPlayerMatchSummaryDisplay from '@client/vue/utility/csgo/CsgoPlayerMatchSummaryDisplay.vue'
import CsgoVodPicker from '@client/vue/utility/csgo/CsgoVodPicker.vue'
import CsgoRoundTimeline from '@client/vue/utility/csgo/CsgoRoundTimeline.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        CsgoPlayerMatchSummaryDisplay,
        CsgoVodPicker,
        CsgoRoundTimeline,
        MatchShareButton,
        MatchFavoriteButton,
    }
})
export default class CsgoMatch extends Vue {
    SquadOvGames: any = SquadOvGames
    match: CsgoFullMatchData | null = null

    @Prop({type:Number, required: true})
    userId!: number

    @Prop({required: true})
    matchUuid! : string

    vod : VodAssociation | null = null
    theaterMode: boolean = false
    currentPlayerHeight : number = 0
    vodReady: boolean = false
    vodTime: Date | null = null
    enableDraw: boolean = false

    currentRound: number = 0
    selectedMatchUserId: number | null = null

    get matchData(): CsgoFullMatchDataWrapper | null {
        if (!this.match) {
            return null
        }
        return new CsgoFullMatchDataWrapper(this.match)
    }

    get currentSteamId() : number | null {
        if (!this.match) {
            return null
        }
        return this.match.summary.steamId
    }

    @Watch('currentSteamId')
    syncSteamIdToUserId() {
        if (!this.matchData || this.currentSteamId === null) {
            return
        }
        this.selectedMatchUserId = this.matchData.steamIdToUserId(this.currentSteamId)
    }

    @Watch('matchUuid')
    @Watch('userId')
    refreshData() {
        apiClient.getCsgoMatchData(this.matchUuid, this.userId).then((resp: ApiData<CsgoFullMatchData>) => {
            this.match = resp.data
        }).catch((err: any) => {
            console.log('Failed to get CSGO match data: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

#match-vod {
    width: 100%;
}

</style>