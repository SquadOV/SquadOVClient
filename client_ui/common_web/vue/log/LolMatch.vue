<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <!-- Header summary -->
                <div class="d-flex align-center">
                    <lol-match-summary
                        class="flex-grow-1 mb-4"
                        :match="playerSummary"
                        :user-id="userId"
                    >
                    </lol-match-summary>

                    <match-favorite-button
                        :match-uuid="matchUuid"
                    >
                    </match-favorite-button>

                    <match-share-button
                        :match-uuid="matchUuid"
                        :game="SquadOvGames.LeagueOfLegends"
                        :permissions="matchPermissions"
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

                        <lol-vod-picker
                            :vod.sync="vod"
                            :match-uuid="matchUuid"
                            :match="currentMatch.match"
                            :current-participant-id="currentParticipantId"
                            :timestamp="vodTime"
                            disable-favorite
                            :enable-draw.sync="enableDraw"
                        >
                        </lol-vod-picker>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <lol-event-manager
                            :match="currentMatch.match"
                            :timeline="currentMatch.timeline"
                            :current-participant-id="currentParticipantId"
                            :display-events.sync="displayEvents"
                            :has-vod="!!vod && vodReady"
                            :style="eventsStyle"
                            @go-to-timestamp="goToTimestamp(arguments[0], true)"
                        >
                        </lol-event-manager>
                    </v-col>
                </v-row>

                <!-- Alternative match timeline (seconds) -->
                <generic-match-timeline
                    class="full-width my-2"
                    :start="0"
                    :end="currentMatch.match.gameDuration"
                    :current="currentMatchTimeSeconds"
                    :input-events="genericEvents"
                    :major-tick-every="90"
                    @go-to-timestamp="goToTimestampSeconds(arguments[0], false)"
                >
                    <template v-slot:tick="{ tick }">
                        {{ secondsToTimeString(tick) }}
                    </template>

                    <template v-slot:event="{event}">
                        <lol-event-display
                            mini
                            :match="currentMatch.match"
                            :timeline="currentMatch.timeline"
                            :current-participant-id="currentParticipantId"
                            :input-event="event.value"
                            :height="24"
                        >
                        </lol-event-display>
                    </template>
                </generic-match-timeline>

                <!-- Scoreboard and stats -->
                <v-tabs v-model="currentTab">
                    <v-tab>
                        Scoreboard
                    </v-tab>

                    <v-tab>
                        Advanced
                    </v-tab>

                    <v-tab @change="$refs.stats.refreshGraphs()">
                        Timeline
                    </v-tab>
                </v-tabs>

                <lol-match-scoreboard
                    :match="currentMatch.match"
                    :current-participant-id="currentParticipantId"
                    :style="scoreboardStyle"
                >
                </lol-match-scoreboard>

                <lol-match-advanced-stats
                    class="my-1 full-width"
                    :match="currentMatch.match"
                    :current-participant-id="currentParticipantId"
                    :style="statsStyle"
                >
                </lol-match-advanced-stats>

                <lol-stat-timeline-container
                    ref="stats"
                    class="my-1 full-width"
                    :match="currentMatch.match"
                    :timeline="currentMatch.timeline"
                    :current-participant-id="currentParticipantId"
                    :style="timelineStyle"
                    :current-time="currentMatchTimeSeconds"
                    @go-to-timestamp="goToTimestamp"
                >
                </lol-stat-timeline-container>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { FullLolMatch, LolPlayerMatchSummary } from '@client/js/lol/matches'
import { LolParticipant, LolParticipantIdentity } from '@client/js/lol/participant'
import { LolMatchEvent } from '@client/js/lol/timeline'
import { apiClient, ApiData } from '@client/js/api'
import { VodAssociation } from '@client/js/squadov/vod'
import { GenericEvent } from '@client/js/event'
import { secondsToTimeString } from '@client/js/time'
import { computeColorForLolEvent } from '@client/js/lol/color'
import { colorFromElementTheme} from '@client/js/color'
import { SquadOvGames } from '@client/js/squadov/game'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import LolMatchSummary from '@client/vue/utility/lol/LolMatchSummary.vue'
import LolMatchScoreboard from '@client/vue/utility/lol/LolMatchScoreboard.vue'
import LolMatchAdvancedStats from '@client/vue/utility/lol/LolMatchAdvancedStats.vue'
import LolEventManager from '@client/vue/utility/lol/LolEventManager.vue'
import GenericMatchTimeline from '@client/vue/utility/GenericMatchTimeline.vue'
import LolEventDisplay from '@client/vue/utility/lol/LolEventDisplay.vue'
import LolStatTimelineContainer from '@client/vue/utility/lol/LolStatTimelineContainer.vue'
import LolVodPicker from '@client/vue/utility/lol/LolVodPicker.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'
import MatchShareBase from '@client/vue/log/MatchShareBase'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        LolMatchSummary,
        LolMatchScoreboard,
        LolMatchAdvancedStats,
        LolEventManager,
        LolEventDisplay,
        LolStatTimelineContainer,
        LolVodPicker,
        GenericMatchTimeline,
        MatchShareButton,
        MatchFavoriteButton,
    }
})
export default class LolMatch extends mixins(MatchShareBase) {
    SquadOvGames: any = SquadOvGames
    secondsToTimeString = secondsToTimeString

    @Prop()
    userId!: number

    @Prop({required: true})
    matchUuid! : string

    currentMatch: FullLolMatch | null = null
    vod: VodAssociation | null = null
    displayEvents: LolMatchEvent[] = []
    enableDraw: boolean = false

    $refs!: {
        player: VideoPlayer
        stats: LolStatTimelineContainer
    }
    currentPlayerHeight : number = 0
    theaterMode: boolean = false
    currentTab: number = 0
    vodTime: Date | null = null
    vodReady: boolean = false

    goToTimestampSeconds(t: number, useOffset: boolean) {
        this.goToTimestamp(t * 1000.0, useOffset)
    }

    goToTimestamp(t: number, useOffset: boolean) {
        if (!this.vod || !this.currentMatch?.gameStartTime) {
            return
        }

        // Assume t is number of milliseconds since the game started.
        let diffMs = (this.currentMatch.gameStartTime.getTime() - this.vod.startTime.getTime()) + t
        this.$refs.player.goToTimeMs(diffMs, useOffset)
    }

    // This function is needed to convert from the current VOD time to the match time in seconds
    // as we're not guaranteed that the VOD starts as soon as the match starts.
    get currentMatchTimeSeconds(): number {
        if (!this.vodTime || !this.currentMatch?.gameStartTime) {
            return 0
        }

        let diffMs = this.vodTime.getTime() - this.currentMatch.gameStartTime.getTime()
        return Math.max(diffMs / 1000.0, 0.0)
    }

    get currentParticipantId() : number | undefined | null {
        return this.currentMatch?.userIdToParticipantId[this.userId]
    }

    get playerSummary() : LolPlayerMatchSummary | null {
        if (!this.currentMatch) {
            return null
        }
        return {
            matchUuid: this.matchUuid,
            gameCreation: this.currentMatch.match.gameCreation,
            gameDuration: this.currentMatch.match.gameDuration,
            gameType: this.currentMatch.match.gameType,
            gameVersion: this.currentMatch.match.gameVersion,
            queueId: this.currentMatch.match.queueId,
            seasonId: this.currentMatch.match.seasonId,
            mapId: this.currentMatch.match.mapId,
            gameMode: this.currentMatch.match.gameMode,
            currentParticipantId: this.currentParticipantId,
            participants: this.currentMatch.match.participants.map((ele: LolParticipant) => {
                return {
                    participantId: ele.participantId,
                    championId: ele.championId,
                    summonerName: this.currentMatch?.match.participantIdentities.find((p: LolParticipantIdentity) => {
                        return ele.participantId === p.participantId
                    })?.player?.summonerName,
                    teamId: ele.teamId,
                    kills: ele.stats.kills,
                    deaths: ele.stats.deaths,
                    assists: ele.stats.assists,
                    totalDamageDealtToChampions: ele.stats.totalDamageDealtToChampions,
                    totalMinionsKilled: ele.stats.totalMinionsKilled,
                    wardsPlaced: ele.stats.wardsPlaced,
                    lane: ele.timeline.lane,
                    win: ele.stats.win,
                }
            }),
            hasVod: !!this.vod,
        }
    }

    @Watch('matchUuid')
    refreshData() {
        this.currentMatch = null
        this.refreshMatchPermissions(this.matchUuid, SquadOvGames.LeagueOfLegends)
        apiClient.getLolMatch(this.matchUuid).then((resp: ApiData<FullLolMatch>) => {
            this.currentMatch = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain LoL match: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get scoreboardStyle() : any {
        return {
            visibility: this.currentTab === 0 ? 'visible' : 'hidden',
            display: this.currentTab === 0 ? 'block' : 'none'            
        }
    }

    get statsStyle() : any {
        return {
            visibility: this.currentTab === 1 ? 'visible' : 'hidden',
            display: this.currentTab === 1 ? 'table' : 'none'
        }
    }

    get timelineStyle(): any {
        return {
            visibility: this.currentTab === 2 ? 'visible' : 'hidden',
            display: this.currentTab === 2 ? 'table' : 'none'
        }
    }

    get eventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight + 64}px`,
        }
    }

    get genericEvents(): GenericEvent[] {
        if (!this.currentMatch) {
            return []
        }

        return this.displayEvents.map((ele: LolMatchEvent) => {
            return {
                tm: ele.timestamp / 1000,
                value: ele,
                key: ele.type.toLowerCase().replace('_', '-'),
                color: colorFromElementTheme(this.$parent.$el, computeColorForLolEvent(this.currentMatch!.match, ele, this.currentParticipantId))
            }
        })
    }
}

</script>

<style scoped>

#match-vod {
    width: 100%;
}

</style>