<template>
    <loading-container :is-loading="!ready">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <!--
                    Header that summarizes some key details about the final state of the match.
                    Namely, we want to know the two heros, final life count, final turn, match duration.
                    Whether the player won or lost.
                -->
                <v-row>
                    <hearthstone-match-header-display
                        :current-match="matchWrapper"
                    >
                    </hearthstone-match-header-display>
                </v-row>
                
                <!--
                    VOD and navigatable Per-round actions            
                -->
                <v-row>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :video-uuid="videoUuid"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                        >
                        </video-player>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <v-tabs grow v-model="eventSectionTab">
                            <v-tab>
                                Events
                            </v-tab>

                            <v-tab-item>
                                <hearthstone-turn-events-display
                                    :current-match="matchWrapper"
                                    :turn="currentTurn"
                                    :style="roundEventsStyle"
                                    @go-to-event="goToVodTime"
                                    :has-vod="hasVod"
                                >
                                </hearthstone-turn-events-display>
                            </v-tab-item>

                            <template v-if="matchWrapper.isBattlegrounds">
                                <!-- This show the current deck STATE (e.g. hand, graveyard, in play, etc.) -->
                                <v-tab>
                                    Tavern
                                </v-tab>

                                <v-tab-item>
                                    <hearthstone-battleground-tavern-summary
                                        :height="currentPlayerHeight - 48"
                                    >
                                    </hearthstone-battleground-tavern-summary>    
                                </v-tab-item>
                            </template>

                            <template v-else>
                                <!-- This show the current deck state for the start of the given round (e.g. hand, graveyard, in play, etc.) -->
                                <v-tab>
                                    Deck
                                </v-tab>

                                <v-tab-item>
                                    <hearthstone-current-deck-state
                                        :deck="matchWrapper.currentPlayerDeck"
                                        :current-match="matchWrapper"
                                        :turn="currentTurn"
                                        :player-id="matchWrapper.currentPlayerId"
                                        :style="deckStateStyle"
                                    >
                                    </hearthstone-current-deck-state>
                                </v-tab-item>
                            </template>
                        </v-tabs>
                    </v-col>
                </v-row>

                <!--
                    Turn-timeline (mulligan + turns)
                -->
                <v-row>
                    <hearthstone-turn-timeline-display
                        id="turn-timeline"
                        :current-match="matchWrapper"
                        :turn.sync="currentTurn"
                        v-if="!matchWrapper.isBattlegrounds"
                    >
                    </hearthstone-turn-timeline-display>

                    <hearthstone-battlegrounds-turn-timeline-display
                        id="turn-timeline"
                        :current-match="matchWrapper"
                        :turn.sync="currentTurn"
                        v-else
                    >
                    </hearthstone-battlegrounds-turn-timeline-display>
                </v-row>

                <!--
                    Deck Comparison
                -->
                <v-row v-if="!matchWrapper.isBattlegrounds">
                    <v-col cols="6">
                        <template v-if="!!matchWrapper.currentPlayerDeck">
                            <div class="d-flex align-center justify-center pa-4">
                                <span class="text-h5">
                                    {{ matchWrapper.currentPlayerDeck.name }}
                                </span>
                            </div>
                            <v-divider></v-divider>
                            <hearthstone-match-deck-display
                                :deck="matchWrapper.currentPlayerDeck"
                            >
                            </hearthstone-match-deck-display>
                        </template>
                    </v-col>

                    <v-col cols="6">
                        <template v-if="!!matchWrapper.opposingPlayerDeck">
                            <div class="d-flex align-center justify-center pa-4">
                                <span class="text-h5">
                                    {{ matchWrapper.opposingPlayerDeck.name }}
                                </span>
                            </div>
                            <v-divider></v-divider>
                            <hearthstone-match-deck-display
                                :deck="matchWrapper.opposingPlayerDeck"
                            >
                            </hearthstone-match-deck-display>
                        </template>
                    </v-col>
                </v-row>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { HearthstoneMatch as RawHearthstoneMatch, HearthstoneMatchWrapper, HearthstoneMatchLogs} from '@client/js/hearthstone/hearthstone_match'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import HearthstoneMatchHeaderDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchHeaderDisplay.vue'
import HearthstoneTurnEventsDisplay from '@client/vue/utility/hearthstone/HearthstoneTurnEventsDisplay.vue'
import HearthstoneTurnTimelineDisplay from '@client/vue/utility/hearthstone/HearthstoneTurnTimelineDisplay.vue'
import HearthstoneBattlegroundsTurnTimelineDisplay from '@client/vue/utility/hearthstone/HearthstoneBattlegroundsTurnTimelineDisplay.vue'
import HearthstoneMatchDeckDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckDisplay.vue'
import HearthstoneBattlegroundTavernSummary from '@client/vue/utility/hearthstone/HearthstoneBattlegroundTavernSummary.vue'
import HearthstoneCurrentDeckState from '@client/vue/utility/hearthstone/HearthstoneCurrentDeckState.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'

@Component({
  components: {
    LoadingContainer,
    HearthstoneMatchHeaderDisplay,
    HearthstoneTurnEventsDisplay,
    HearthstoneTurnTimelineDisplay,
    HearthstoneBattlegroundsTurnTimelineDisplay,
    HearthstoneMatchDeckDisplay,
    HearthstoneBattlegroundTavernSummary,
    HearthstoneCurrentDeckState,
    VideoPlayer
  }
})
export default class HearthstoneMatch extends Vue {
    @Prop({required: true})
    matchId!: string

    @Prop({required: true})
    userId!: number

    eventSectionTab: number = 0

    // VOD display
    $refs!: {
        player: VideoPlayer
    }
    theaterMode: boolean = false
    currentPlayerHeight : number = 0
    vod: VodAssociation | null = null

    // Match Info
    currentMatch: RawHearthstoneMatch | null = null
    currentTurn: number = 0

    // Match loading
    ready: boolean = false

    get videoUuid() : string | undefined {
        return this.vod?.videoUuid
    }

    get roundEventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight - 48}px`,
        }
    }

    get deckStateStyle() : any {
        let style = this.roundEventsStyle
        return {
            'overflow': 'auto',
            ...style
        }
    }

    get matchWrapper() : HearthstoneMatchWrapper {
        return new HearthstoneMatchWrapper(this.currentMatch!)
    }

    get hasVod() : boolean {
        return !!this.vod
    }

    goToVodTime(tm : Date) {
        if (!this.hasVod) {
            return
        }

        let diffMs = tm.getTime() - this.vod!.startTime.getTime()
        this.$refs.player.goToTimeMs(diffMs)
    }

    @Watch('matchId')
    @Watch('userId')
    refreshData() {
        this.ready = false
        this.currentMatch = null
        apiClient.getHearthstoneMatch(this.matchId, this.userId).then((resp : ApiData<RawHearthstoneMatch>) => {
            this.currentMatch = resp.data

            apiClient.getHearthstoneMatchLogs(this.matchId, this.userId).then((resp: ApiData<HearthstoneMatchLogs>) => {
                this.matchWrapper.addLogs(resp.data)
                this.ready = true
            }).catch((err: any) => {
                console.log('Failed to obtain Hearthstone match logs: ', err)    
            })
        }).catch((err: any) => {
            console.log('Failed to obtain Hearthstone match: ', err)
        })
    }

    @Watch('matchId')
    @Watch('userId')
    refreshVod() {
        this.vod = null
        apiClient.findVodFromMatchUserId(this.matchId, this.userId).then((resp : ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err : any) => {
            this.vod = null
        })
    }

    mounted() {
        this.refreshData()
        this.refreshVod()
    }
}

</script>

<style scoped>

#turn-timeline {
    width: 100%;
}

</style>