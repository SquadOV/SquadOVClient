<template>
    <div class="round-container d-flex">
        <v-btn
            :class="`round-button flex-grow-1 flex-shrink-1 ${ (turn == 0) ? 'selected-round' : '' }`"
            tile
            :style="mulliganStyling"
            @click="goToMulligan"
        >
            <span class="text-center">Mulligan</span>
        </v-btn>

        <v-btn
            v-for="t in numTurns"
            :key="t"
            tile
            :class="`round-button flex-grow-1 flex-shrink-1 ${ (t == turn) ? 'selected-round' : '' }`"
            :style="turnStyling(t)"
            @click="goToTurn(t)"
        >
            <span class="text-center">{{ t }}</span>
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { Color } from '@client/js/color'
import { getSameTeamColor, getOpposingTeamColor } from '@client/js/hearthstone/hearthstone_colors'

// Display num turns + 1 (mulligan) rounds.
@Component
export default class HearthstoneTurnTimelineDisplay extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    @Prop({type: Number, required: true})
    turn!: number

    get numTurns(): number {
        return this.currentMatch.numTurns
    }

    goToMulligan() {
        this.goToTurn(0)
    }

    goToTurn(turn : number) {
        this.$emit('update:turn', turn)
    }

    get mulliganStyling() : any {
        return this.turnStyling(0)
    }

    turnStyling(rnd : number) : any {
        let accentColor : Color = { r : 0, g : 0, b : 0}
        if (rnd == 0) {
            accentColor.r = 25
            accentColor.g = 118
            accentColor.b = 210
        } else {
            if (this.currentMatch.isTurnForCurrentPlayer(rnd)) {
                accentColor = getSameTeamColor()
            } else {
                accentColor = getOpposingTeamColor()   
            }
        }

        return {
            'border-top': `2px solid rgb(${accentColor.r}, ${accentColor.g}, ${accentColor.b})`
        }
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

.selected-round {
    background-color: rgba(255, 255, 0, 0.5) !important;
}

</style>