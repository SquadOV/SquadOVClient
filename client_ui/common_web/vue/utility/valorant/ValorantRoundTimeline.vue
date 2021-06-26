<template>
    <div class="round-container d-flex">
        <v-btn
            v-for="rnd in totalRounds"
            :key="rnd"
            tile
            :class="`round-button flex-grow-1 flex-shrink-1 ${ (rnd == (round + 1)) ? 'selected-round' : '' }`"
            :style="roundStyling(rnd - 1)"
            @click="goToRound(rnd - 1)"
        >
            <div class="d-flex flex-column justify-center align-center">
                <div>{{ rnd }}</div>
                <div v-if="!!currentPlayer">{{ roundWins[rnd-1] }} - {{ roundLoss[rnd-1] }}</div>
            </div>
        </v-btn>
    </div>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import {
    ValorantMatchRoundWrapper,
    ValorantMatchDetailsWrapper,
    ValorantMatchPlayerWrapper
} from '@client/js/valorant/valorant_matches_parsed'
import { Color } from '@client/js/color'
import CommonComponent from '@client/vue/CommonComponent'

@Component
export default class ValorantRoundTimeline extends mixins(CommonComponent) {
    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop({type: Number, required: true})
    round!: number

    @Prop()
    currentPlayer! : ValorantMatchPlayerWrapper | null

    get totalRounds() : number {
        return this.match.getNumRounds()
    }

    goToRound(r : number) {
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchInfo, this.AnalyticsAction.GoToPhase, 'Round', r)
        this.$emit('update:round', r)
    }

    get roundWins(): number[] {
        let ret: number[] = []
        let cnt = 0
        for (let i = 0; i < this.totalRounds; ++i) {
            ret.push(cnt)

            if (!this.currentPlayer) {
                continue
            }

            let rnd = this.match.getRound(i)
            if (!rnd) {
                continue
            }

            if (rnd._r.winningTeam === this.currentPlayer._p.teamId) {
                cnt += 1
            }
        }
        return ret
    }

    get roundLoss(): number[] {
        let ret: number[] = []
        let cnt = 0
        for (let i = 0; i < this.totalRounds; ++i) {
            ret.push(cnt)

            if (!this.currentPlayer) {
                continue
            }

            let rnd = this.match.getRound(i)
            if (!rnd) {
                continue
            }

            if (rnd._r.winningTeam !== this.currentPlayer._p.teamId) {
                cnt += 1
            }
        }
        return ret
    }

    roundStyling(rnd : number) : any {
        let round = this.match.getRound(rnd)
        if (!round) {
            return {}
        }

        let winner = round._r.winningTeam
        let accentColor : string
        if (!!this.currentPlayer) {
            if (winner == this.currentPlayer._p.teamId) {
                accentColor = 'color-friendly'
            } else {
                accentColor = 'color-enemy'
            }
        } else {
            if (winner == 'Blue') {
                accentColor = 'color-blue-team'
            } else if (winner == 'Red') {
                accentColor = 'color-red-team'
            } else {
                accentColor = 'color-neutral'
            }
        }

        let ret: any = {
            'border-top': `2px solid var(--${accentColor})`
        }
        return ret
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