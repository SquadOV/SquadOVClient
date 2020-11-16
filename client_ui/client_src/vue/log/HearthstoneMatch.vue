<template>
    <loading-container :is-loading="!currentMatch">
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
                        <hearthstone-turn-events-display
                            :style="roundEventsStyle"
                        >
                        </hearthstone-turn-events-display>
                    </v-col>
                </v-row>

                <!--
                    Turn-timeline (mulligan + turns)
                -->
                <v-row class="my-4">
                    <hearthstone-turn-timeline-display>
                    </hearthstone-turn-timeline-display>
                </v-row>
                
                <!--
                    Deck Comparison
                -->
                <v-row>
                    <v-col cols="6">
                        <hearthstone-match-deck-display>
                        </hearthstone-match-deck-display>
                    </v-col>

                    <v-col cols="6">
                        <hearthstone-match-deck-display>
                        </hearthstone-match-deck-display>
                    </v-col>
                </v-row>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { HearthstoneMatch as RawHearthstoneMatch, HearthstoneMatchWrapper} from '@client/js/hearthstone/hearthstone_match'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import HearthstoneMatchHeaderDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchHeaderDisplay.vue'
import HearthstoneTurnEventsDisplay from '@client/vue/utility/hearthstone/HearthstoneTurnEventsDisplay.vue'
import HearthstoneTurnTimelineDisplay from '@client/vue/utility/hearthstone/HearthstoneTurnTimelineDisplay.vue'
import HearthstoneMatchDeckDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckDisplay.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'

@Component({
  components: {
    LoadingContainer,
    HearthstoneMatchHeaderDisplay,
    HearthstoneTurnEventsDisplay,
    HearthstoneTurnTimelineDisplay,
    HearthstoneMatchDeckDisplay,
    VideoPlayer
  }
})
export default class HearthstoneMatch extends Vue {
    @Prop({required: true})
    matchId!: string

    // VOD display
    $refs!: {
        player: VideoPlayer
    }
    theaterMode: boolean = false
    currentPlayerHeight : number = 0

    vod: VodAssociation | null = null
    currentMatch: RawHearthstoneMatch | null = null

    get videoUuid() : string | undefined {
        return this.vod?.videoUuid
    }

    get roundEventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight}px`,
        }
    }

    get matchWrapper() : HearthstoneMatchWrapper {
        return new HearthstoneMatchWrapper(this.currentMatch!)
    }

    refreshData() {
        apiClient.getHearthstoneMatch(this.matchId).then((resp : ApiData<RawHearthstoneMatch>) => {
            this.currentMatch = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain Hearthstone match: ', err)
        })
    }

    refreshVod() {
        apiClient.findVodFromMatchUserUuid(this.matchId, this.$store.state.currentUser!.uuid).then((resp : ApiData<VodAssociation>) => {
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