<template>
    <div class="full-width">
        <v-sheet
            rounded
            :style="divStyle"
        >
            <v-row no-gutters>
                <v-col cols="1" align-self="center">
                    <div class="d-flex justify-center text-h2">
                        <span :style="currentPlayerHpStyle">{{ currentPlayerLife }}</span>
                    </div>
                </v-col>
                <v-col cols="2" align-self="center">
                    <hearthstone-hero-display
                        :hero-card="currentPlayerHeroCard"
                        :max-height="150"
                    >
                    </hearthstone-hero-display>

                    <div class="d-flex justify-center my-1">
                        <span>
                            {{ currentPlayerName }}
                        </span>
                    </div>
                </v-col>

                <v-col cols="1" align-self="center">
                    <hearthstone-player-medal-display
                        v-if="currentMatch.isRanked"
                        :medal-info="currentPlayerMedals"
                        :max-height="100"
                    >
                    </hearthstone-player-medal-display>
                </v-col>

                <v-col cols="4" align-self="center">
                    <div class="d-flex justify-center">
                        {{ dateTime }}
                    </div>

                    <div class="d-flex justify-center">
                        {{ gameMode }}
                    </div>

                    <div class="d-flex justify-center">
                        {{ matchLength }} - {{ numTurns }} Turns
                    </div>
                </v-col>

                <v-col cols="1" align-self="center">
                    <hearthstone-player-medal-display
                        v-if="currentMatch.isRanked"
                        :medal-info="opposingPlayerMedals"
                        :max-height="100"
                    >
                    </hearthstone-player-medal-display>
                </v-col>

                <v-col cols="2" align-self="center">
                    <hearthstone-hero-display
                        :hero-card="opposingPlayerHeroCard"
                        :max-height="150"
                    >
                    </hearthstone-hero-display>

                    <div class="d-flex justify-center my-1">
                        <span>
                            {{ opposingPlayerName }}
                        </span>
                    </div>
                </v-col>
                <v-col cols="1" align-self="center">
                    <div class="d-flex justify-center text-h2">
                        <span :style="opposingPlayerHpStyle">{{ opposingPlayerLife }}</span>
                    </div>
                </v-col>
            </v-row>
        </v-sheet>
    </div>
</template>

<script lang="ts">


import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { HearthstonePlayState } from '@client/js/hearthstone/hearthstone_playstate'
import { HearthstoneMedalInfo } from '@client/js/hearthstone/hearthstone_player'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'
import HearthstonePlayerMedalDisplay from '@client/vue/utility/hearthstone/HearthstonePlayerMedalDisplay.vue'

@Component({
    components: {
        HearthstoneHeroDisplay,
        HearthstonePlayerMedalDisplay
    }
})
export default class HearthstoneMatchHeaderDisplay extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    computeWLColor(w: boolean) : string {
        return w ? '#4CAF50' : '#FF5252'
    }

    get winLossColor() : string {
        return this.computeWLColor(this.currentPlayerWon)
    }

    get opposingPlayerHpColor() : string {
        return this.computeWLColor(!this.currentPlayerWon)
    }

    get divStyle() : any {
        return {
            'border-left': `5px solid ${this.winLossColor}`,
        }
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

    get dateTime() : string {
        return standardFormatTime(this.currentMatch.matchTime)
    }

    get gameMode():  string {
        return this.currentMatch.displayedGameMode
    }

    get numTurns(): number {
        return this.currentMatch.numTurns
    }

    get matchLength() : string {
        return secondsToTimeString(this.currentMatch.elapsedSeconds)
    }

    get currentPlayerWon(): boolean {
        let entity = this.currentMatch.currentPlayerEntity
        return entity?.playState == HearthstonePlayState.Won
    }

    get currentPlayerLife(): number {
        let entity = this.currentMatch.currentPlayerHeroEntity
        let hp = entity?.remainingHealth
        return !!hp ? hp : 0
    }

    get currentPlayerMedals(): HearthstoneMedalInfo | undefined {
        if (!this.currentMatch.currentPlayerId) {
            return undefined
        }
        return this.currentMatch.medalsForPlayer(this.currentMatch.currentPlayerId)
    }

    get currentPlayerHeroCard():  string | undefined {
        if (!this.currentMatch.currentPlayerId) {
            return undefined
        }
        return this.currentMatch.playerHeroCard(this.currentMatch.currentPlayerId)
    }

    get currentPlayerName() : string | undefined {
        return this.currentMatch.currentPlayer?.name
    }

    get opposingPlayerHeroCard():  string | undefined {
        if (!this.currentMatch.opposingPlayerId) {
            return undefined
        }
        return this.currentMatch.playerHeroCard(this.currentMatch.opposingPlayerId)
    }

    get opposingPlayerName() : string | undefined {
        return this.currentMatch.opposingPlayer?.name
    }

    get opposingPlayerLife(): number {
        let entity = this.currentMatch.opposingPlayerHeroEntity
        let hp = entity?.remainingHealth
        return !!hp ? hp : 0
    }

    get opposingPlayerMedals(): HearthstoneMedalInfo | undefined {
        if (!this.currentMatch.opposingPlayerId) {
            return undefined
        }
        return this.currentMatch.medalsForPlayer(this.currentMatch.opposingPlayerId)
    }
}


</script>