<template>
    <!-- Display username/game, VOD preview, and match summary -->
    <div class="full-width">
        <div class="d-flex align-center">
            <v-img
                class="mr-4"
                :src="$root.generateAssetUri(gameToIcon(match.base.game))"
                max-width="24px"
                width="24px"
                max-height="24px"
                height="24px"
                contain
            >
            </v-img>

            <div class="text-center text-caption">
                {{ match.base.username }} played...
            </div>

            <v-spacer></v-spacer>

            <template v-if="!!match.base.favoriteReason">
                <div class="text-right text-caption">
                    {{ match.base.favoriteReason }}
                </div>

                <v-tooltip bottom>
                    <template v-slot:activator="{on, attrs}">
                        <v-icon
                            v-bind="attrs"
                            v-on="on"
                            color="warning"
                        >
                            mdi-star
                        </v-icon>
                    </template>

                    {{ match.base.favoriteReason }}
                </v-tooltip>
            </template>

            <v-icon v-if="match.base.isWatchlist">
                mdi-playlist-check
            </v-icon>
        </div>

        <div
            class="d-flex align-center full-width"
            @mouseover="onHover"
            @mouseout="onLeave"
        >
            <video-preview-player
                v-if="!disablePreview && !forceNoPreview"
                :vod="match.base.vod"
                :use-local-vod="useLocalVodPreview"
                :access-token="match.base.accessToken"
                class="recent-match-item preview-item"
                ref="player"
            >
            </video-preview-player>

            <aimlab-task-summary-display
                class="flex-grow-1 recent-match-item"
                v-if="!!match.aimlabTask && match.base.game == SquadOvGames.AimLab"
                :task="match.aimlabTask"
                :user-id="match.base.userId"
                fill
                :disable-click="disableClick"
                :access-token="match.base.accessToken"
            >
            </aimlab-task-summary-display>

            <lol-match-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.lolMatch && match.base.game == SquadOvGames.LeagueOfLegends"
                :match="match.lolMatch"
                :user-id="match.base.userId"
                :mini="!disableMini"
                fill
                :disable-click="disableClick"
                :access-token="match.base.accessToken"
            >
            </lol-match-summary>

            <tft-match-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.tftMatch && match.base.game == SquadOvGames.TeamfightTactics"
                :match="match.tftMatch"
                :user-id="match.base.userId"
                :puuid="match.tftMatch.puuid"
                :mini="!disableMini"
                fill
                :disable-click="disableClick"
                :access-token="match.base.accessToken"
            >
            </tft-match-summary>

            <valorant-player-match-summary-display
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.valorantMatch && match.base.game == SquadOvGames.Valorant"
                :match="match.valorantMatch"
                :user-id="match.base.userId"
                :account="match.valorantMatch.puuid"
                fill
                :mini="!disableMini"
                :disable-click="disableClick"
                :access-token="match.base.accessToken"
            >
            </valorant-player-match-summary-display>

            <wow-keystone-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.wowChallenge && match.base.game == SquadOvGames.WorldOfWarcraft"
                :challenge="match.wowChallenge"
                :user-id="match.base.userId"
                :mini="!disableMini"
                fill
                :disable-link="disableClick"
                :access-token="match.base.accessToken"
            >
            </wow-keystone-summary>

            <wow-encounter-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.wowEncounter && match.base.game == SquadOvGames.WorldOfWarcraft"
                :encounter="match.wowEncounter"
                :user-id="match.base.userId"
                :mini="!disableMini"
                fill
                :disable-link="disableClick"
                :access-token="match.base.accessToken"
            >
            </wow-encounter-summary>

            <wow-arena-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.wowArena && match.base.game == SquadOvGames.WorldOfWarcraft"
                :arena="match.wowArena"
                :user-id="match.base.userId"
                :mini="!disableMini"
                fill
                :disable-link="disableClick"
                :access-token="match.base.accessToken"
            >
            </wow-arena-summary>

            <csgo-player-match-summary-display
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.csgoMatch && match.base.game == SquadOvGames.Csgo"
                :match="match.csgoMatch"
                :user-id="match.base.userId"
                :mini="!disableMini"
                fill
                :disable-link="disableClick"
                :access-token="match.base.accessToken"
            >
            </csgo-player-match-summary-display>

            <hearthstone-match-summary-display
                class="flex-grow-1 recent-match-item"
                v-else-if="match.base.game == SquadOvGames.Hearthstone"
                :match-id="match.base.matchUuid"
                :user-id="match.base.userId"
                :mini="!disableMini"
                fill
                :disable-click="disableClick"
                :access-token="match.base.accessToken"
            >
            </hearthstone-match-summary-display>

            <upload-progress-display
                class="ml-2"
                v-if="showUploadProgress"
                :video-uuid="match.base.vod.videoTracks[0].metadata.videoUuid"
            >
            </upload-progress-display>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { RecentMatch } from '@client/js/squadov/recentMatch'
import { SquadOvGames, gameToIcon } from '@client/js/squadov/game'
import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'
import HearthstoneMatchSummaryDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchSummaryDisplay.vue'
import LolMatchSummary from '@client/vue/utility/lol/LolMatchSummary.vue'
import TftMatchSummary from '@client/vue/utility/tft/TftMatchSummary.vue'
import ValorantPlayerMatchSummaryDisplay from '@client/vue/utility/valorant/ValorantPlayerMatchSummaryDisplay.vue'
import WowKeystoneSummary from '@client/vue/utility/wow/WowKeystoneSummary.vue'
import WowEncounterSummary from '@client/vue/utility/wow/WowEncounterSummary.vue'
import WowArenaSummary from '@client/vue/utility/wow/WowArenaSummary.vue'
import CsgoPlayerMatchSummaryDisplay from '@client/vue/utility/csgo/CsgoPlayerMatchSummaryDisplay.vue'
import VideoPreviewPlayer from '@client/vue/utility/VideoPreviewPlayer.vue'
import UploadProgressDisplay from '@client/vue/utility/squadov/UploadProgressDisplay.vue'

@Component({
    components: {
        AimlabTaskSummaryDisplay,
        HearthstoneMatchSummaryDisplay,
        LolMatchSummary,
        TftMatchSummary,
        ValorantPlayerMatchSummaryDisplay,
        WowKeystoneSummary,
        WowEncounterSummary,
        WowArenaSummary,
        VideoPreviewPlayer,
        CsgoPlayerMatchSummaryDisplay,
        UploadProgressDisplay,
    }
})
export default class RecentMatchDisplay extends Vue {
    SquadOvGames = SquadOvGames
    gameToIcon = gameToIcon

    @Prop({required: true})
    match!: RecentMatch

    @Prop({type: Boolean, default: false})
    disableClick!: boolean

    @Prop({type: Boolean, default: false})
    disablePreview!: boolean

    @Prop({type: Boolean, default: false})
    disableMini!: boolean

    @Prop({type: Boolean, default: false})
    useLocalVodPreview!: boolean

    @Prop({type: Boolean, default: false})
    showUploadProgress!: boolean

    forceNoPreview: boolean = false

    $refs!: {
        player: VideoPreviewPlayer
    }

    onHover() {
        if (!this.match.base.isLocal || this.useLocalVodPreview) {
            this.$refs.player.startPlay()
        }
    }

    onLeave() {
        if (!this.match.base.isLocal || this.useLocalVodPreview) {
            this.$refs.player.pausePlay()
        }
    }

    destroyPreview() {
        if (!!this.$refs.player) {
            this.$refs.player.$destroy()
            this.forceNoPreview = true
        }
    }
}

</script>

<style scoped>

.recent-match-item {
    height: 125px;
    min-height: 125px;
    max-height: 125px;
}

.preview-item {
    width: 200px;
    min-width: 200px;
    max-width: 200px;
}

</style>