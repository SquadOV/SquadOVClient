<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex align-center">
                    <tft-match-summary
                        class="flex-grow-1 mb-4"
                        :match="playerSummary"
                        :puuid="puuid"
                        :user-id="userId"
                    >
                    </tft-match-summary>

                    <match-favorite-button
                        :match-uuid="matchUuid"
                    >
                    </match-favorite-button>

                    <match-share-button
                        :match-uuid="matchUuid"
                        :game="SquadOvGames.TeamfightTactics"
                        :permissions="matchPermissions"
                        :full-path="$route.fullPath"
                        :timestamp="timestamp"
                        :user-id="userId"
                        :is-local="!!vod ? vod.isLocal : false"
                    >
                    </match-share-button>
                </div>

                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            class="mb-4"
                            :vod="vod"
                            id="task-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                            :current-time.sync="vodTime"
                            :enable-draw="enableDraw"
                            :current-ts.sync="timestamp"
                            :match-uuid="matchUuid"
                            :game="SquadOvGames.TeamfightTactics"
                            :permissions="matchPermissions"
                            :full-path="$route.fullPath"
                            :timestamp="timestamp"
                            :user-id="userId"
                            :is-local="!!vod ? vod.isLocal : false"
                            :player-height.sync="currentPlayerHeight"
                        >
                        </video-player>

                        <tft-vod-picker
                            class="full-width"
                            :match-uuid="matchUuid"
                            :puuid="puuid"
                            :vod.sync="vod"
                            :match="currentMatch"
                            :timestamp="vodTime"
                            disable-favorite
                            :enable-draw.sync="enableDraw"
                        >
                        </tft-vod-picker>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <generic-match-sidebar
                            :match-uuid="matchUuid"
                            :video-uuid="vod.videoUuid"
                            hide-events
                            :style="eventsStyle"
                            :current-tm="vodTime"
                            v-if="!!currentMatch && !!vod"
                            :start-tm="currentMatch.data.info.gameDatetime"
                            @go-to-time="goToVodTime(arguments[0])"
                        >
                        </generic-match-sidebar>
                    </v-col>
                </v-row>

                <v-row>
                    <v-col cols="12">
                        <tft-scoreboard
                            :puuid="puuid"
                            :match="currentMatch"
                            :patch="patch"
                        >
                        </tft-scoreboard>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { WrappedTftMatch, TftPlayerMatchSummary, TftParticipant, getTftSetNumber } from '@client/js/tft/matches'
import { VodAssociation } from '@client/js/squadov/vod'
import { SquadOvGames } from '@client/js/squadov/game'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import TftMatchSummary from '@client/vue/utility/tft/TftMatchSummary.vue'
import TftScoreboard from '@client/vue/utility/tft/TftScoreboard.vue'
import TftVodPicker from '@client/vue/utility/tft/TftVodPicker.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'
import MatchShareBase from '@client/vue/log/MatchShareBase'
import GenericMatchSidebar from '@client/vue/utility/GenericMatchSidebar.vue'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        TftMatchSummary,
        TftScoreboard,
        TftVodPicker,
        MatchShareButton,
        MatchFavoriteButton,
        GenericMatchSidebar,
    }
})
export default class TftMatch extends mixins(MatchShareBase) {
    SquadOvGames: any = SquadOvGames

    @Prop({required: true})
    puuid!: string

    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    currentMatch: WrappedTftMatch | null = null
    vodTime: Date | null = null
    enableDraw: boolean = false
    theaterMode: boolean = false
    currentPlayerHeight : number = 0

    $refs!: {
        player: VideoPlayer
    }

    get participantMap(): Map<string, TftParticipant> {
        let ret = new Map()
        if (!!this.currentMatch) {
            for (let p of this.currentMatch.data.info.participants) {
                ret.set(p.puuid, p)
            }
        }
        return ret
    }

    get patch(): string {
        if (!this.currentMatch) {
            return ''
        }
        return getTftSetNumber(this.currentMatch.data.info.tftSetNumber, this.currentMatch.data.info.gameVersion)
    }

    get playerSummary(): TftPlayerMatchSummary | null {
        if (!this.currentMatch) {
            return null
        }

        let participant = this.participantMap.get(this.puuid)
        if (!participant) {
            return null
        }

        return {
            matchUuid: this.matchUuid,
            gameDatetime: this.currentMatch.data.info.gameDatetime,
            gameLength: this.currentMatch.data.info.gameLength,
            gameVariation: this.currentMatch.data.info.gameVariation,
            gameVersion: this.currentMatch.data.info.gameVersion,
            queueId: this.currentMatch.data.info.queueId,
            tftSetNumber: this.currentMatch.data.info.tftSetNumber,
            companion: participant.companion,
            level: participant.level,
            placement: participant.placement,
            traits: participant.traits,
            units: participant.units,
            hasVod: !!this.vod,
            puuid: this.puuid,
        }
    }

    @Watch('matchUuid')
    refreshData() {
        this.refreshMatchPermissions(this.matchUuid, SquadOvGames.TeamfightTactics)
        apiClient.accessToken().getTftMatch(this.matchUuid).then((resp: ApiData<WrappedTftMatch>) => {
            this.currentMatch = resp.data
        }).catch((err: any) => {
            console.error('Failed to get TFT match: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get eventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight + this.vodPickerHeight}px`,
        }
    }

    goToVodTime(tm : Date) {
        if (!this.vod) {
            return
        }
        
        let diffMs = tm.getTime() - this.vod.startTime.getTime()
        this.$refs.player.goToTimeMs(diffMs, false)
    }
}

</script>

<style scoped>

#task-vod {
    width: 100%;
}

</style>