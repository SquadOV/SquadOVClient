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
        </div>

        <div
            class="d-flex align-center full-width"
            @mouseover="onHover"
            @mouseout="onLeave"
        >
            <video-preview-player
                :vod="match.base.vod"
                class="recent-match-item preview-item"
                ref="player"
            >
            </video-preview-player>

            <aimlab-task-summary-display
                class="flex-grow-1 recent-match-item"
                v-if="!!match.aimlabTask"
                :task="match.aimlabTask"
                :user-id="match.base.userId"
                fill
            >
            </aimlab-task-summary-display>

            <lol-match-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.lolMatch"
                :match="match.lolMatch"
                :user-id="match.base.userId"
            >
            </lol-match-summary>

            <tft-match-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.tftMatch"
                :match="match.tftMatch"
                :user-id="match.base.userId"
                :puuid="match.tftMatch.puuid"
            >
            </tft-match-summary>

            <valorant-player-match-summary-display
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.valorantMatch"
                :match="match.valorantMatch"
                :user-id="match.base.userId"
                :account="match.valorantMatch.puuid"
                fill
                mini
            >
            </valorant-player-match-summary-display>

            <wow-keystone-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.wowChallenge"
                :challenge="match.wowChallenge"
                :user-id="match.base.userId"
                mini
                fill
            >
            </wow-keystone-summary>

            <wow-encounter-summary
                class="flex-grow-1 recent-match-item"
                v-else-if="!!match.wowEncounter"
                :encounter="match.wowEncounter"
                :user-id="match.base.userId"
                mini
                fill
            >
            </wow-encounter-summary>

            <hearthstone-match-summary-display
                class="flex-grow-1 recent-match-item"
                v-else
                :match-id="match.base.matchUuid"
                :user-id="match.base.userId"
                mini
                fill
            >
            </hearthstone-match-summary-display>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { RecentMatch } from '@client/js/squadov/recentMatch'
import { gameToIcon } from '@client/js/squadov/game'
import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'
import HearthstoneMatchSummaryDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchSummaryDisplay.vue'
import LolMatchSummary from '@client/vue/utility/lol/LolMatchSummary.vue'
import TftMatchSummary from '@client/vue/utility/tft/TftMatchSummary.vue'
import ValorantPlayerMatchSummaryDisplay from '@client/vue/utility/valorant/ValorantPlayerMatchSummaryDisplay.vue'
import WowKeystoneSummary from '@client/vue/utility/wow/WowKeystoneSummary.vue'
import WowEncounterSummary from '@client/vue/utility/wow/WowEncounterSummary.vue'
import VideoPreviewPlayer from '@client/vue/utility/VideoPreviewPlayer.vue'

@Component({
    components: {
        AimlabTaskSummaryDisplay,
        HearthstoneMatchSummaryDisplay,
        LolMatchSummary,
        TftMatchSummary,
        ValorantPlayerMatchSummaryDisplay,
        WowKeystoneSummary,
        WowEncounterSummary,
        VideoPreviewPlayer,
    }
})
export default class RecentMatchDisplay extends Vue {
    gameToIcon = gameToIcon

    @Prop({required: true})
    match!: RecentMatch

    $refs!: {
        player: VideoPreviewPlayer
    }

    onHover() {
        this.$refs.player.startPlay()
    }

    onLeave() {
        this.$refs.player.pausePlay()
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