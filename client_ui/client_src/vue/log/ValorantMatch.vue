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

                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :video-uuid="videoUuid"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                        >
                        </video-player>

                        <valorant-vod-pov-picker
                            :puuid.sync="povPuuid"
                            :vod.sync="vod"
                            :player-metadata.sync="playerMetadata"
                            :match="matchWrapper"
                            :ref-puuid="puuid"
                        >
                        </valorant-vod-pov-picker>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <valorant-round-events
                            :match="matchWrapper"
                            :round="currentRound"
                            :force-disable-go-to-event="!hasMatchTiming"
                            :style="roundEventsStyle"
                            :current-player="currentPlayer"
                            :metadata="playerMetadata"
                            @go-to-event="goToVodTime"
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
                            @go-to-event="goToVodTime"
                        >
                        </valorant-match-player-card>
                    </v-col>
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
import { VodAssociation } from '@client/js/squadov/vod'
import { ValorantMatchDetails, ValorantMatchPlayerMatchMetadata } from '@client/js/valorant/valorant_matches'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchRoundWrapper,
    ValorantMatchPlayerWrapper,
    ValorantMatchTeamWrapper,
} from '@client/js/valorant/valorant_matches_parsed'

import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantMatchHeaderSummary from '@client/vue/utility/valorant/ValorantMatchHeaderSummary.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import ValorantRoundEvents from '@client/vue/utility/valorant/ValorantRoundEvents.vue'
import ValorantRoundTimeline from '@client/vue/utility/valorant/ValorantRoundTimeline.vue'
import ValorantTeamRoundDisplay from '@client/vue/utility/valorant/ValorantTeamRoundDisplay.vue'
import ValorantFullMatchScoreboard from '@client/vue/utility/valorant/ValorantFullMatchScoreboard.vue'
import ValorantMatchPlayerCard from '@client/vue/utility/valorant/ValorantMatchPlayerCard.vue'
import ValorantVodPovPicker from '@client/vue/utility/valorant/ValorantVodPovPicker.vue'

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
        ValorantVodPovPicker
    }
})
export default class ValorantMatch extends Vue {
    @Prop()
    puuid! : string | null

    @Prop()
    userId!: number

    povPuuid: string | null = null
    vod : VodAssociation | null = null
    playerMetadata: ValorantMatchPlayerMatchMetadata | null = null

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

    get videoUuid() : string | undefined {
        return this.vod?.videoUuid
    }

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
        return this.currentMatch.teams.length
    }

    get primaryTeam() : ValorantMatchTeamWrapper | null {
        if (!this.currentMatch) {
            return null
        }

        if (!this.currentPlayer) {
            return this.matchWrapper!.getTeam('Blue')
        } else {
            return this.matchWrapper!.getPlayerTeam(this.currentPlayer._p.subject)
        }
        return null
    }

    get secondaryTeam() : ValorantMatchTeamWrapper | null {
        if (!this.currentMatch) {
            return null
        }

        if (!this.currentPlayer) {
            return this.matchWrapper!.getTeam('Red')
        } else {
            return this.matchWrapper!.getOpposingPlayerTeam(this.currentPlayer._p.subject)
        }
        return null
    }

    goToVodTime(tm : Date) {
        if (!this.hasMatchTiming) {
            return
        }
        
        let diffMs = tm.getTime() - this.playerMetadata!.startTime.getTime()
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