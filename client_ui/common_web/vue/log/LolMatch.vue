<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <!-- Header summary -->
                <lol-match-summary
                    class="full-width"
                    :match="playerSummary"
                    :puuid="puuid"
                    :user-id="userId"
                >
                </lol-match-summary>

                <!-- VOD + Event List -->
                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :vod="vod"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                        >
                        </video-player>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <lol-event-manager
                            :match="currentMatch.match"
                            :timeline="currentMatch.timeline"
                            :current-participant-id="currentParticipantId"
                            :display-events.sync="displayEvents"
                            :has-vod="!!vod"
                            :style="eventsStyle"
                            @go-to-timestamp="goToTimestamp"
                        >
                        </lol-event-manager>
                    </v-col>
                </v-row>

                <!-- Alternative match timeline (seconds) -->
                <generic-match-timeline
                    class="full-width my-2"
                    :start="0"
                    :end="currentMatch.match.gameDuration"
                    :current="0"
                    :input-events="genericEvents"
                    :major-tick-every="90"
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
                >
                </lol-stat-timeline-container>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { FullLolMatch, LolPlayerMatchSummary } from '@client/js/lol/matches'
import { LolParticipant, LolParticipantIdentity } from '@client/js/lol/participant'
import { LolMatchEvent } from '@client/js/lol/timeline'
import { apiClient, ApiData } from '@client/js/api'
import { VodAssociation } from '@client/js/squadov/vod'
import { GenericEvent } from '@client/js/event'
import { secondsToTimeString } from '@client/js/time'
import { computeColorForLolEvent } from '@client/js/lol/color'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import LolMatchSummary from '@client/vue/utility/lol/LolMatchSummary.vue'
import LolMatchScoreboard from '@client/vue/utility/lol/LolMatchScoreboard.vue'
import LolMatchAdvancedStats from '@client/vue/utility/lol/LolMatchAdvancedStats.vue'
import LolEventManager from '@client/vue/utility/lol/LolEventManager.vue'
import GenericMatchTimeline from '@client/vue/utility/GenericMatchTimeline.vue'
import LolEventDisplay from '@client/vue/utility/lol/LolEventDisplay.vue'
import LolStatTimelineContainer from '@client/vue/utility/lol/LolStatTimelineContainer.vue'

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
        GenericMatchTimeline,
    }
})
export default class LolMatch extends Vue {
    secondsToTimeString = secondsToTimeString

    @Prop()
    puuid! : string | null

    @Prop()
    userId!: number

    @Prop({required: true})
    matchUuid! : string

    currentMatch: FullLolMatch | null = null
    vod: VodAssociation | null = null
    displayEvents: LolMatchEvent[] = []

    $refs!: {
        player: VideoPlayer
        stats: LolStatTimelineContainer
    }
    currentPlayerHeight : number = 0
    theaterMode: boolean = false
    currentTab: number = 0

    goToTimestamp(t: number) {

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
                color: computeColorForLolEvent(this.currentMatch!.match, ele, this.currentParticipantId)
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