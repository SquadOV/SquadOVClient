<template>
    <v-sheet
        class="match-summary"
        rounded
        :style="style"
    >
        <v-row no-gutters>
            <v-col cols="2" align-self="center">
                <valorant-agent-icon
                    :agent="match.agentId"
                    :width-height="100"
                    :patch="match.patchId"
                >
                </valorant-agent-icon>
            </v-col>

            <v-col cols="2" align-self="center">
                <p :style="wlColorStyle">
                    {{ match.myTeamScore }} - {{ match.otherTeamScore }}
                </p>

                <v-chip :style="csRankStyle">{{ csRank }}</v-chip>
            </v-col>

            <v-col cols="2" align-self="center">
                <valorant-hit-tracker
                    :headshots="match.headshots"
                    :bodyshots="match.bodyshots"
                    :legshots="match.legshots"
                >
                </valorant-hit-tracker>
            </v-col>
        </v-row>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'
import { getOrdinal } from '@client/js/ordinal'

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantHitTracker from '@client/vue/utility/valorant/ValorantHitTracker.vue'

@Component({
    components: {
        ValorantAgentIcon,
        ValorantHitTracker
    }
})
export default class ValorantPlayerMatchSummaryDisplay extends Vue {
    @Prop({required: true})
    match! : ValorantPlayerMatchSummary

    get csRank() : string {
        return getOrdinal(this.match.csRank)
    }

    get csRankStyle() : any {
        let color = '#424242'
        if (this.match.csRank == 1) {
            color = '#AF9500'
        } else if (this.match.csRank == 2) {
            color = '#6C7A86'
        } else if (this.match.csRank == 3) {
            color = '#AD8A56'
        }
        return {
            'background-color': color
        }
    }

    get winLossColor() : string {
        let color : string = this.match.win ? '#4CAF50' : '#FF5252'
        return color
    }

    get style() : any {
        return {
            'border-left': `5px solid ${this.winLossColor}`,
        }
    }

    get wlColorStyle() : any {
        return {
            'color': `${this.winLossColor}`
        }
    }
}

</script>

<style scoped>

.match-summary {
    width: 100%;
}

</style>