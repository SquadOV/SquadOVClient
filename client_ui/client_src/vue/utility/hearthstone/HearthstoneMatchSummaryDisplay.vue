<template>
    <div>
        <router-link :to="to">
            <v-sheet
                class="task-summary"
                rounded
                :style="divStyle"
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
                    <v-col cols="2" align-self="center" class="text-body-2">
                        <p>
                            {{ dateTime }}
                        </p>

                        <p>
                            {{ gameMode }}
                        </p>

                        <p>
                            {{ matchLength }} - {{ numTurns }} Turns
                            <template v-if="!matchWrapper.isBattlegrounds">
                                (<span :style="currentPlayerHpStyle">{{ currentPlayerLife }}</span> - <span :style="opposingPlayerHpStyle">{{ opposingPlayerLife }}</span>)
                            </template>
                        </p>
                    </v-col>

                    <!-- Rank (if relevant) -->
                    <v-col cols="1" align-self="center">
                        <hearthstone-player-medal-display
                            v-if="isRanked && !matchWrapper.isBattlegrounds"
                            :medal-info="medalInfo"
                            :max-height="100"
                        >
                        </hearthstone-player-medal-display>
                        
                        <div class="d-flex justify-center text-h3" v-else-if="matchWrapper.isBattlegrounds">
                            <hearthstone-battlegrounds-game-leaderboard-place-display
                                class="mx-2"
                                :current-match="matchWrapper"
                            >
                            </hearthstone-battlegrounds-game-leaderboard-place-display>
                        </div>
                    </v-col>

                    <!-- Board State -->
                    <v-col :cols="matchWrapper.isBattlegrounds ? 7 : 5" align-self="center">
                        <hearthstone-mini-board-state-display
                            :snapshot="latestSnapshot"
                            :player-match-id="localPlayerMatchId"
                        >
                        </hearthstone-mini-board-state-display>
                    </v-col>

                    <!-- Enemy hero -->
                    <v-col cols="2" align-self="center" v-if="!matchWrapper.isBattlegrounds">
                        <hearthstone-hero-display
                            :hero-card="enemyHeroCard"
                            :max-height="150"
                        >
                        </hearthstone-hero-display>
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
import { HearthstoneMatch, HearthstoneMatchWrapper, constructGameTypeString, HearthstoneFormatType, isGameTypeConstructed, isGameTypeRanked } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneMedalInfo, HearthstonePlayer } from '@client/js/hearthstone/hearthstone_player'
import { HearthstoneEntity, HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneMatchSnapshot } from '@client/js/hearthstone/hearthstone_snapshot'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import { HearthstoneCardtype } from '@client/js/hearthstone/hearthstone_cardtype'
import { HearthstonePlayState } from '@client/js/hearthstone/hearthstone_playstate'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'
import HearthstonePlayerMedalDisplay from '@client/vue/utility/hearthstone/HearthstonePlayerMedalDisplay.vue'
import HearthstoneMiniBoardStateDisplay from '@client/vue/utility/hearthstone/HearthstoneMiniBoardStateDisplay.vue'
import HearthstoneBattlegroundsGameLeaderboardPlaceDisplay from '@client/vue/utility/hearthstone/HearthstoneBattlegroundsGameLeaderboardPlaceDisplay.vue'

@Component({
    components: {
        HearthstoneHeroDisplay,
        HearthstonePlayerMedalDisplay,
        HearthstoneMiniBoardStateDisplay,
        HearthstoneBattlegroundsGameLeaderboardPlaceDisplay
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

    get matchWrapper(): HearthstoneMatchWrapper {
        return new HearthstoneMatchWrapper(this.currentMatch!)
    }

    get currentPlayerLife(): number {
        let entity = this.matchWrapper.currentPlayerHeroEntity
        let hp = entity?.remainingHealth
        return !!hp ? hp : 0
    }

    get opposingPlayerLife(): number {
        let entity = this.matchWrapper.opposingPlayerHeroEntity
        let hp = entity?.remainingHealth
        return !!hp ? hp : 0
    }

    get deckHeroCard() : string | undefined  {
        if (!this.matchWrapper.currentPlayerId) {
            return undefined
        }
        return this.matchWrapper.playerHeroCard(this.matchWrapper.currentPlayerId)
    }

    get enemyHeroCard() : string | undefined {
        if (!this.latestSnapshot) {
            return undefined
        }

        for (let [eid, entity] of Object.entries(this.latestSnapshot.entities)) {
            let typedEntity: HearthstoneEntity = entity
            let wrappedEntity = new HearthstoneEntityWrapper(typedEntity)
            if (wrappedEntity.controller != this.localPlayerMatchId 
                && wrappedEntity.cardType == HearthstoneCardtype.Hero) {
                return wrappedEntity.cardId
            }
        }
        return undefined
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

    get localPlayerMatchId() : number | undefined {
        for (let [pid, player] of Object.entries(this.currentMatch!.metadata.players)) {
            let typedPlayer: HearthstonePlayer = player
            if (!typedPlayer.local) {
                continue
            }
            return parseInt(pid)
        }
        return undefined
    }

    get localPlayer() : HearthstonePlayer | undefined {
        if (!this.localPlayerMatchId) {
            return undefined
        }
        return this.currentMatch!.metadata.players[this.localPlayerMatchId]
    }

    get medalInfo() : HearthstoneMedalInfo | undefined {
        if (this.currentMatch!.metadata.formatType == HearthstoneFormatType.Standard) {
            return this.localPlayer?.medalInfo.standard
        } else {
            return this.localPlayer?.medalInfo.wild
        }
    }

    get matchLength() : string {
        return secondsToTimeString(this.currentMatch!.metadata.elapsedSeconds)
    }

    get numTurns() : number {
        let numTurns = this.latestSnapshot?.auxData?.currentTurn
        return !!numTurns ? numTurns : 0
    }

    get latestSnapshot(): HearthstoneMatchSnapshot | null {
        if (!this.currentMatch) {
            return null
        }
        return this.currentMatch.latestSnapshot
    }

    get won() : boolean {
        if (!this.latestSnapshot || !this.localPlayerMatchId) {
            return false
        }

        if (this.matchWrapper.isBattlegrounds) {
            let place = this.matchWrapper.currentPlayerHeroEntity?.leaderboardPlace
            return !!place ? place <= 4 : false
        } else {
            let entityId = this.latestSnapshot.playerIdToEntityId[this.localPlayerMatchId]
            let entity = new HearthstoneEntityWrapper(this.latestSnapshot.entities[entityId])
            return entity.playState == HearthstonePlayState.Won
        }
    }

    computeWLColor(w: boolean) : string {
        return w ? '#4CAF50' : '#FF5252'
    }

    get winLossColor() : string {
        return this.computeWLColor(this.won)
    }

    get opposingPlayerHpColor() : string {
        return this.computeWLColor(!this.won)
    }


    get currentPlayerHpStyle() : any {
        return {
            'color': this.winLossColor
        }
    }

    get opposingPlayerHpStyle() : any {
        return {
            'color': this.opposingPlayerHpColor
        }
    }

    get divStyle() : any {
        return {
            'border-left': `5px solid ${this.winLossColor}`,
        }
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

.task-summary {
    width: 100%;
    position: relative;
}

</style>