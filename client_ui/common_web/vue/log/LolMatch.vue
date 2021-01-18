<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <!-- Header summary -->
                <v-row>
                    <v-col cols="12">
                        <lol-match-summary
                            class="full-width"
                            :match="playerSummary"
                            :puuid="puuid"
                            :user-id="userId"
                        >
                        </lol-match-summary>
                    </v-col>
                </v-row>

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
                    </v-col>
                </v-row>

                <!-- Alternative match timeline -->
                <v-row>
                    <v-col cols="12">
                    </v-col>
                </v-row>

                <!-- Scoreboard and stats -->
                <v-row>
                    <v-col cols="12">
                        <v-tabs>
                            <v-tab>
                                Scoreboard
                            </v-tab>

                            <v-tab-item>
                                <lol-match-scoreboard
                                    :match="currentMatch.match"
                                    :current-participant-id="currentParticipantId"
                                >
                                </lol-match-scoreboard>
                            </v-tab-item>

                            <v-tab>
                                Advanced
                            </v-tab>

                            <v-tab-item>
                            </v-tab-item>

                            <v-tab>
                                Timeline
                            </v-tab>

                            <v-tab-item>
                            </v-tab-item>
                        </v-tabs>
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
import { FullLolMatch, LolPlayerMatchSummary } from '@client/js/lol/matches'
import { LolParticipant, LolParticipantIdentity } from '@client/js/lol/participant'
import { apiClient, ApiData } from '@client/js/api'
import { VodAssociation } from '@client/js/squadov/vod'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import LolMatchSummary from '@client/vue/utility/lol/LolMatchSummary.vue'
import LolMatchScoreboard from '@client/vue/utility/lol/LolMatchScoreboard.vue'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        LolMatchSummary,
        LolMatchScoreboard
    }
})
export default class LolMatch extends Vue {
    @Prop()
    puuid! : string | null

    @Prop()
    userId!: number

    @Prop({required: true})
    matchUuid! : string

    currentMatch: FullLolMatch | null = null
    vod: VodAssociation | null = null

    $refs!: {
        player: VideoPlayer
    }
    currentPlayerHeight : number = 0
    theaterMode: boolean = false

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
                    totalDamageDealt: ele.stats.totalDamageDealt,
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
}

</script>

<style scoped>

#match-vod {
    width: 100%;
}

</style>