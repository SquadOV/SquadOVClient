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

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import {
    ValorantMatchRoundWrapper,
    ValorantMatchDetailsWrapper,
    ValorantMatchPlayerWrapper
} from '@client/js/valorant/valorant_matches_parsed'
import {
    getBlueTeamColor,
    getRedTeamColor,
    getSameTeamColor
} from '@client/js/valorant/valorant_colors'
import { Color } from '@client/js/color'

@Component
export default class ValorantRoundTimeline extends Vue {
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
        let accentColor : Color = { r : 0, g : 0, b : 0}
        if (!!this.currentPlayer) {
            if (winner == this.currentPlayer._p.teamId) {
                accentColor = getSameTeamColor()
            } else {
                accentColor = getRedTeamColor()
            }
        } else {
            if (winner == 'Blue') {
                accentColor = getBlueTeamColor()
            } else if (winner == 'Red') {
                accentColor = getRedTeamColor()
            }
        }

        let ret: any = {
            'border-top': `2px solid rgb(${accentColor.r}, ${accentColor.g}, ${accentColor.b})`
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