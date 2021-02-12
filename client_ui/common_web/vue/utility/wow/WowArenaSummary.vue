<template>
    <wow-generic-match-summary
        :match="arena"
        :user-id="userId"
        :elapsed-time="elapsedTime"
        :mini="mini"
        :fill="fill"
        use-teams
        :friendly-team="friendlyTeam"
    >
        <template v-slot="{ instanceName }">
            <div class="text-h6">
                {{ instanceName }} ({{ arena.type }})
            </div>

            <div class="d-flex align-center text-subtitle-2" :style="ratingColorStyle">
                <v-icon class="mr-1" color="success" x-small v-if="arena.success">
                    mdi-arrow-up-bold
                </v-icon>

                <v-icon class="mr-1" color="error" x-small v-else>
                    mdi-arrow-down-bold
                </v-icon>
                {{ rating }}
            </div>
        </template>
    </wow-generic-match-summary>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import WowGenericMatchSummary from '@client/vue/utility/wow/WowGenericMatchSummary.vue'
import { Prop } from 'vue-property-decorator'
import { WowArena, getOppositeWowArenaTeam } from '@client/js/wow/matches'
import { secondsToTimeString } from '@client/js/time'
import {
    getGenericWinColor,
    getGenericLossColor,
    colorToCssString
} from '@client/js/color'

@Component({
    components: {
        WowGenericMatchSummary,
    }
})
export default class WowArenaSummary extends Vue {
    @Prop({required: true})
    arena!: WowArena

    @Prop({type: Number, required: true})
    userId!: number

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    get friendlyTeam(): number {
        if (this.arena.winningTeamId === null) {
            return 0
        }
        return this.arena.success ? this.arena.winningTeamId : getOppositeWowArenaTeam(this.arena.winningTeamId)
    }

    get ratingColorStyle() : any {
        let color = this.arena.success ? 
            getGenericWinColor() :
            getGenericLossColor()
        return {
            'color': colorToCssString(color)
        }
    }

    get rating(): number {
        if (!this.arena.newRatings) {
            return 0
        }
        return this.arena.newRatings[this.friendlyTeam]
    }

    get elapsedTime(): string {
        let finishTime: Date = !!this.arena.finishTime ?
            this.arena.finishTime : new Date()

        return secondsToTimeString((finishTime.getTime() - this.arena.tm.getTime()) / 1000)
    }
}

</script>