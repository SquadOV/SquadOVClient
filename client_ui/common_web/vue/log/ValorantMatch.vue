<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex align-center">
                    <valorant-match-header-summary
                        class="flex-grow-1 mb-4"
                        :match="matchWrapper"
                        :current-player="puuid"
                    >
                    </valorant-match-header-summary>

                    <match-favorite-button
                        :match-uuid="matchUuid"
                    >
                    </match-favorite-button>

                    <match-share-button
                        :match-uuid="matchUuid"
                        :game="SquadOvGames.Valorant"
                        :user-id="userId"
                        :permissions="matchPermissions"
                    >
                    </match-share-button>
                </div>

                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :vod="vod"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                            :ready.sync="vodReady"
                            :current-time.sync="vodTime"
                            :enable-draw="enableDraw"
                        >
                        </video-player>

                        <valorant-vod-pov-picker
                            :match-uuid="matchUuid"
                            :puuid.sync="povPuuid"
                            :vod.sync="vod"
                            :player-metadata.sync="playerMetadata"
                            :match="matchWrapper"
                            :ref-puuid="puuid"
                            :timestamp="vodTime"
                            disable-favorite
                            :enable-draw.sync="enableDraw"
                        >
                        </valorant-vod-pov-picker>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <valorant-round-events
                            :match="matchWrapper"
                            :round="currentRound"
                            :force-disable-go-to-event="!hasMatchTiming || !vodReady"
                            :style="roundEventsStyle"
                            :current-player="currentPlayer"
                            :metadata="playerMetadata"
                            @go-to-event="goToVodTime(arguments[0], true)"
                        >
                        </valorant-round-events>
                    </v-col>
                </v-row>

                <v-row class="my-4">
                    <valorant-round-timeline
                        id="round-timeline"
                        :match="matchWrapper"
                        :round.sync="currentRoundNum"
                        :current-player="currentPlayer"
                    >
                    </valorant-round-timeline>
                </v-row>

                <v-row no-gutters v-if="numTeams == 2 && !!primaryTeam && !!secondaryTeam && !!currentRound" class="mb-4">
                    <v-col cols="6">
                        <valorant-team-round-display
                            :match="matchWrapper"
                            :round="currentRound"
                            :current-player="currentPlayer"
                            :team="primaryTeam"
                        >
                        </valorant-team-round-display>
                    </v-col>

                    <v-col cols="6">
                        <valorant-team-round-display
                            :match="matchWrapper"
                            :round="currentRound"
                            :current-player="currentPlayer"
                            :team="secondaryTeam"
                            mirror
                        >
                        </valorant-team-round-display>
                    </v-col>
                </v-row>

                <v-row>
                    <v-col :cols="!!currentPlayer ? 6 : 12">
                        <valorant-full-match-scoreboard
                            :match="matchWrapper"
                            :current-player.sync="currentPlayer"
                        >
                        </valorant-full-match-scoreboard>
                    </v-col>

                    <v-col cols="6" v-if="!!currentPlayer">
                        <valorant-match-player-card
                            :match="matchWrapper"
                            :current-player="currentPlayer"
                            :metadata="playerMetadata"
                            :force-disable-go-to-event="!hasMatchTiming"
                            @go-to-event="goToVodTime(arguments[0], true)"
                        >
                        </valorant-match-player-card>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { VodAssociation } from '@client/js/squadov/vod'
import { ValorantMatchDetails, ValorantMatchPlayerMatchMetadata } from '@client/js/valorant/valorant_matches'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchRoundWrapper,
    ValorantMatchPlayerWrapper,
    ValorantMatchTeamWrapper,
} from '@client/js/valorant/valorant_matches_parsed'
import { SquadOvGames } from '@client/js/squadov/game'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantMatchHeaderSummary from '@client/vue/utility/valorant/ValorantMatchHeaderSummary.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import ValorantRoundEvents from '@client/vue/utility/valorant/ValorantRoundEvents.vue'
import ValorantRoundTimeline from '@client/vue/utility/valorant/ValorantRoundTimeline.vue'
import ValorantTeamRoundDisplay from '@client/vue/utility/valorant/ValorantTeamRoundDisplay.vue'
import ValorantFullMatchScoreboard from '@client/vue/utility/valorant/ValorantFullMatchScoreboard.vue'
import ValorantMatchPlayerCard from '@client/vue/utility/valorant/ValorantMatchPlayerCard.vue'
import ValorantVodPovPicker from '@client/vue/utility/valorant/ValorantVodPovPicker.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'
import MatchShareBase from '@client/vue/log/MatchShareBase'

@Component({
    components: {
        LoadingContainer,
        ValorantMatchHeaderSummary,
        VideoPlayer,
        ValorantRoundEvents,
        ValorantRoundTimeline,
        ValorantTeamRoundDisplay,
        ValorantFullMatchScoreboard,
        ValorantMatchPlayerCard,
        ValorantVodPovPicker,
        MatchShareButton,
        MatchFavoriteButton,
    }
})
export default class ValorantMatch extends mixins(MatchShareBase) {
    SquadOvGames: any = SquadOvGames
    
    @Prop()
    puuid! : string | null

    @Prop()
    userId!: number

    povPuuid: string | null = null
    vod : VodAssociation | null = null
    playerMetadata: ValorantMatchPlayerMatchMetadata | null = null

    @Prop({required: true})
    matchUuid! : string

    $refs!: {
        player: VideoPlayer
    }
    currentPlayerHeight : number = 0

    currentMatch : ValorantMatchDetails | null = null
    currentPlayer : ValorantMatchPlayerWrapper | null = null

    currentRoundNum : number = 0

    theaterMode: boolean = false
    vodReady: boolean = false
    vodTime: Date | null = null
    enableDraw: boolean = false
    
    get matchWrapper() : ValorantMatchDetailsWrapper | null {
        if (!this.currentMatch) {
            return null
        }
        return new ValorantMatchDetailsWrapper(this.currentMatch)
    }

    get currentRound() : ValorantMatchRoundWrapper | null {
        if (!this.matchWrapper) {
            return null
        }

        return this.matchWrapper.getRound(this.currentRoundNum)
    }

    get hasMatchTiming() : boolean {
        return !!this.playerMetadata && !!this.vod
    }

    get roundEventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight + 64}px`,
        }
    }

    get numTeams() : number {
        if (!this.currentMatch) {
            return 0
        }
        return this.currentMatch.data.teams.length
    }

    get primaryTeam() : ValorantMatchTeamWrapper | null {
        if (!this.matchWrapper) {
            return null
        }

        if (!this.currentPlayer) {
            return this.matchWrapper.getTeam('Blue')
        } else {
            return this.matchWrapper.getPlayerTeam(this.currentPlayer._p.puuid)
        }
        return null
    }

    get secondaryTeam() : ValorantMatchTeamWrapper | null {
        if (!this.matchWrapper) {
            return null
        }

        if (!this.currentPlayer) {
            return this.matchWrapper.getTeam('Red')
        } else {
            return this.matchWrapper.getOpposingPlayerTeam(this.currentPlayer._p.puuid)
        }
        return null
    }

    goToVodTime(tm : Date, useOffset: boolean) {
        if (!this.hasMatchTiming) {
            return
        }
        
        let diffMs = tm.getTime() - this.playerMetadata!.startTime.getTime()
        this.$refs.player.goToTimeMs(diffMs, useOffset)
    }

    @Watch('puuid')
    @Watch('currentMatch')
    refreshCurrentPlayer() {
        if (!this.matchWrapper || !this.puuid) {
            this.currentPlayer = null
            return
        }
        this.currentPlayer = this.matchWrapper.getPlayer(this.puuid)
    }

    @Watch('matchUuid')
    refreshMatch() {
        this.refreshMatchPermissions(this.matchUuid, SquadOvGames.Valorant)
        apiClient.getValorantMatchDetails(this.matchUuid).then((resp : ApiData<ValorantMatchDetails>) => {
            this.currentMatch = resp.data
        }).catch((err : any) => {
            console.log('Failed to get valorant match details: ' + err)
        })
    }

    mounted() {
        this.povPuuid = this.puuid
        this.refreshMatch()
    }
}
</script>

<style scoped>

#match-vod {
    width: 100%;
}

#round-timeline {
    width: 100%;
}

</style>