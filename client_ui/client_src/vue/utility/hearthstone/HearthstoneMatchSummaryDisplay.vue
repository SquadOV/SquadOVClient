<template>
    <div>
        <router-link :to="to">
            <v-sheet
                class="task-summary"
                rounded
            >
                <v-row no-gutters v-if="!!currentMatch">
                    <!-- Hero class + Deck Name (if applicable) -->
                    <v-col cols="2" align-self="center">
                        <hearthstone-hero-display
                            :hero-card="deckHeroCard"
                            :max-height="150"
                        >
                        </hearthstone-hero-display>

                        <div class="d-flex justify-center my-1" v-if="isConstructed">
                            <span>
                                {{ deckName }}
                            </span>
                        </div>
                    </v-col>

                    <!-- DateTime + Game Mode -->
                    <v-col cols="3" align-self="center">
                        <p>
                            {{ dateTime }}
                        </p>

                        <p>
                            {{ gameMode }}
                        </p>
                    </v-col>

                    <!-- Rank (if relevant) -->
                    <v-col cols="1" align-self="center">
                        <hearthstone-player-medal-display
                            v-if="isRanked"
                            :medal-info="medalInfo"
                            :max-height="100"
                        >
                        </hearthstone-player-medal-display>
                    </v-col>

                    <!-- Board State -->
                    <v-col cols="5" align-self="center">
                        <hearthstone-mini-board-state-display
                            :snapshot="latestSnapshot"
                        >
                        </hearthstone-mini-board-state-display>
                    </v-col>

                    <!-- Basic Stats about Match -->
                    <v-col cols="1" align-self="center">
                        <p>
                            {{ matchLength }}
                        </p>

                        <p>
                            {{ numTurns }} Turns
                        </p>
                    </v-col>
                </v-row>

                <v-row justify="center" v-else>
                    <v-progress-circular indeterminate size=64></v-progress-circular>
                </v-row>

                <div class="vod-div" v-if="hasVod">
                    <v-icon color="black">
                        mdi-video
                    </v-icon>
                </div>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import * as pi from '@client/js/pages'
import { HearthstoneMatch, constructGameTypeString, HearthstoneFormatType, isGameTypeConstructed, isGameTypeRanked } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneMedalInfo, HearthstonePlayer } from '@client/js/hearthstone/hearthstone_player'
import { HearthstoneMatchSnapshot } from '@client/js/hearthstone/hearthstone_snapshot'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'
import HearthstonePlayerMedalDisplay from '@client/vue/utility/hearthstone/HearthstonePlayerMedalDisplay.vue'
import HearthstoneMiniBoardStateDisplay from '@client/vue/utility/hearthstone/HearthstoneMiniBoardStateDisplay.vue'

@Component({
    components: {
        HearthstoneHeroDisplay,
        HearthstonePlayerMedalDisplay,
        HearthstoneMiniBoardStateDisplay
    }
})
export default class HearthstoneMatchSummaryDisplay extends Vue {
    @Prop({required: true})
    matchId!: string
    vod: VodAssociation | null = null
    currentMatch: HearthstoneMatch | null = null

    get to() : any {
        return {
            name: pi.HearthstoneMatchPageId,
            params: {
                matchId: this.matchId
            }
        }
    }

    get hasVod() : boolean {
        return !!this.vod
    }

    get deckHeroCard() : string | undefined  {
        return this.currentMatch?.metadata.deck?.heroCard
    }

    get deckName() : string | undefined {
        return this.currentMatch?.metadata.deck?.name
    }

    get dateTime() : string {
        return standardFormatTime(this.currentMatch!.metadata.matchTime)
    }

    get gameMode() : string {
        return constructGameTypeString(this.currentMatch!.metadata.gameType, this.currentMatch!.metadata.formatType)
    }

    get isConstructed() : boolean {
        return isGameTypeConstructed(this.currentMatch!.metadata.gameType)
    }

    get isRanked() : boolean {
        return isGameTypeRanked(this.currentMatch!.metadata.gameType)
    }

    get medalInfo() : HearthstoneMedalInfo | undefined {
        for (let [pid, player] of Object.entries(this.currentMatch!.metadata.players)) {
            let typedPlayer: HearthstonePlayer = player
            if (!typedPlayer.local) {
                continue
            }

            if (this.currentMatch!.metadata.formatType == HearthstoneFormatType.Standard) {
                return typedPlayer.medalInfo.standard
            } else {
                return typedPlayer.medalInfo.wild
            }
        }
        return undefined
    }

    get matchLength() : string {
        return secondsToTimeString(this.currentMatch!.metadata.elapsedSeconds)
    }

    get numTurns() : number {
        let numTurns = this.latestSnapshot?.auxData?.currentTurn
        return !!numTurns ? numTurns : 0
    }

    get latestSnapshot(): HearthstoneMatchSnapshot | null {
        return this.currentMatch!.latestSnapshot
    }

    refreshData() {
        apiClient.getHearthstoneMatch(this.matchId).then((resp : ApiData<HearthstoneMatch>) => {
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

<style scoped>

.match-summary {
    width: 100%;
    position: relative;
}

.match-div {
    min-height: 125px;
}

</style>