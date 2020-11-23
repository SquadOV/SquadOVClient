<template>
    <div class="round-container d-flex">
        <div
            class="flex-grow-1 flex-shrink-1"
            v-for="r in numRounds"
            :key="r"
        >
            <v-btn
                tile
                class="round-button round-top-button flex-grow-1 flex-shrink-1"
                :style="roundStyling(r)"
            >
                <span>{{ r }}</span>
            </v-btn>

            <div class="d-flex flex-grow-1 flex-shrink-1">
                <v-btn
                    tile
                    :class="`turn-button round-button flex-grow-1 flex-shrink-1 ${ (r * 2 - 1 == turn) ? 'selected-round' : '' }`"
                    @click="goToTurn(r * 2 - 1)"
                >
                    <v-icon small>
                        mdi-cart
                    </v-icon>
                </v-btn>

                <v-btn
                    tile
                    :class="`turn-button round-button flex-grow-1 flex-shrink-1 ${ (r * 2 == turn) ? 'selected-round' : '' }`"
                    @click="goToTurn(r * 2)"
                >
                    <v-icon small>
                        mdi-sword-cross
                    </v-icon>
                </v-btn>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { BattlegroundsRoundWinState } from '@client/js/hearthstone/hearthstone_snapshot'
import { Color } from '@client/js/color'
import { getSameTeamColor, getOpposingTeamColor, getNeutralTeamColor } from '@client/js/hearthstone/hearthstone_colors'

// Display num turns + 1 (mulligan) rounds.
@Component
export default class HearthstoneBattlegroundsTurnTimelineDisplay extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    @Prop({type: Number, required: true})
    turn!: number

    // A round is composed of two "turns".
    // A "buy" turn and a "play" turn.
    get numRounds(): number {
        return this.currentMatch.numTurns / 2
    }

    goToTurn(turn : number) {
        this.$emit('update:turn', turn)
    }

    roundStyling(rnd : number) : any {
        let accentColor : Color = getOpposingTeamColor()
        
        // For battlegrounds we highlight based off whether or not we won the attack turn.
        let turn = rnd * 2
        let snapshot = this.currentMatch.snapshotForTurn(turn)
        let hero = this.currentMatch.currentPlayerHeroEntity 
        if (!!snapshot && !!hero) {
            let ws = snapshot.battlegroundsWinState(hero._entity.entityId)
            if (ws == BattlegroundsRoundWinState.Win) {
                accentColor = getSameTeamColor()
            } else if (ws == BattlegroundsRoundWinState.Draw) {
                accentColor = getNeutralTeamColor()
            }
        }

        return {
            'border-top': `2px solid rgb(${accentColor.r}, ${accentColor.g}, ${accentColor.b})`,
            'width': '100%'
        }
    }

    mounted() {
        // No turn 0 (mulligan) for battlegrounds
        this.goToTurn(1)
    }
}

</script>

<style scoped>

.round-container {
    border-top: 1px solid rgba(255, 255, 255, 0.5);
    border-bottom: 1px solid rgba(255, 255, 255, 0.5);
}

.round-button {
    border-left: 1px solid rgba(255, 255, 255, 0.5);
    border-right: 1px solid rgba(255, 255, 255, 0.5);
    min-width: 0px !important;
}

.round-top-button {
    border-bottom: 1px solid rgba(255, 255, 255, 0.5);
}

.selected-round {
    background-color: rgba(255, 255, 0, 0.5) !important;
}

.turn-button {
    padding: 0 !important;
}

</style>