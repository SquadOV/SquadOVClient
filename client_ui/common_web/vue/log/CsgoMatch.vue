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
                        <csgo-event-round-display
                            :match="matchData"
                            :match-user-id="selectedMatchUserId"
                            :current-round="currentRound"
                            :style="roundEventsStyle"
                            @go-to-event="goToVodTime(arguments[0])"
                        >
                        </csgo-event-round-display>
                    </v-col>
                </v-row>

                <!-- Round Timeline -->
                <v-row class="mt-4">
                    <csgo-round-timeline
                        :match="matchData"
                        :match-user-id="selectedMatchUserId"
                        :current-round.sync="currentRound"
                    >
                    </csgo-round-timeline>
                </v-row>

                <!-- Round Scoreboard -->
                <v-row no-gutters class="mt-4">
                    <v-col cols="6">
                        <csgo-team-round-scoreboard
                            :match="matchData"
                            :current-round="currentRound"
                            :match-user-id="selectedMatchUserId"
                            :team="currentTeam"
                        >
                        </csgo-team-round-scoreboard>
                    </v-col>

                    <v-col cols="6">
                        <csgo-team-round-scoreboard
                            :match="matchData"
                            :current-round="currentRound"
                            :match-user-id="selectedMatchUserId"
                            :team="oppositeTeam"
                            mirror
                        >
                        </csgo-team-round-scoreboard>
                    </v-col>
                </v-row>

                <!-- Match Scoreboard and player card -->
                <v-row class="mt-4">
                    <v-col cols="6">
                        <csgo-match-scoreboard
                            :match="matchData"
                            :match-user-id.sync="selectedMatchUserId"
                        >
                        </csgo-match-scoreboard>
                    </v-col>

                    <v-col cols="6">
                        <csgo-player-match-card
                            :match="matchData"
                            :match-user-id="selectedMatchUserId"
                            @go-to-event="goToVodTime(arguments[0])"
                        >
                        </csgo-player-match-card>
                    </v-col>
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
import { CsgoFullMatchData, CsgoFullMatchDataWrapper, CsgoEventRoundWrapper } from '@client/js/csgo/match'
import { CsgoTeam, getCsgoOppositeTeam } from '@client/js/csgo/events'
import { VodAssociation } from '@client/js/squadov/vod'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import CsgoPlayerMatchSummaryDisplay from '@client/vue/utility/csgo/CsgoPlayerMatchSummaryDisplay.vue'
import CsgoVodPicker from '@client/vue/utility/csgo/CsgoVodPicker.vue'
import CsgoRoundTimeline from '@client/vue/utility/csgo/CsgoRoundTimeline.vue'
import CsgoEventRoundDisplay from '@client/vue/utility/csgo/CsgoEventRoundDisplay.vue'
import CsgoTeamRoundScoreboard from '@client/vue/utility/csgo/CsgoTeamRoundScoreboard.vue'
import CsgoMatchScoreboard from '@client/vue/utility/csgo/CsgoMatchScoreboard.vue'
import CsgoPlayerMatchCard from '@client/vue/utility/csgo/CsgoPlayerMatchCard.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        CsgoPlayerMatchSummaryDisplay,
        CsgoVodPicker,
        CsgoRoundTimeline,
        CsgoEventRoundDisplay,
        CsgoTeamRoundScoreboard,
        CsgoMatchScoreboard,
        CsgoPlayerMatchCard,
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

    $refs!: {
        player: VideoPlayer
    }

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

    get roundEventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight + 64}px`,
        }
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

    @Watch('matchData')
    onNewMatch() {
        if (!this.matchData || this.matchData.rounds.length == 0) {
            return
        }
        this.currentRound = this.matchData.rounds[0].roundNum
    }

    mounted() {
        this.refreshData()
    }

    get currentTeam(): CsgoTeam {
        if (this.selectedMatchUserId === null || !this.currentRoundData) {
            return CsgoTeam.Spectate
        }
        return this.currentRoundData.userTeam(this.selectedMatchUserId)
    }

    get oppositeTeam(): CsgoTeam {
        return getCsgoOppositeTeam(this.currentTeam)
    }

    get currentRoundData(): CsgoEventRoundWrapper | undefined {
        if (!this.matchData) {
            return undefined
        }
        return this.matchData.round(this.currentRound)
    }

    goToVodTime(tm : Date) {
        if (!this.vod) {
            return
        }
        
        let diffMs = tm.getTime() - this.vod.startTime.getTime()
        if (!!this.match) {
            diffMs += this.match.clockOffsetMs
        }

        this.$refs.player.goToTimeMs(diffMs, true)
    }
}

</script>

<style scoped>

#match-vod {
    width: 100%;
}

</style>