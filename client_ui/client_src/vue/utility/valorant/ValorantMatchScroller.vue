<template>
    <div>
        <valorant-player-match-summary-display
            class="mb-4"
            v-for="match in matches"
            :key="match.matchId"
            :match="match"
        >
        </valorant-player-match-summary-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'
import ValorantPlayerMatchSummaryDisplay from '@client/vue/utility/valorant/ValorantPlayerMatchSummaryDisplay.vue'

@Component({
    components: {
        ValorantPlayerMatchSummaryDisplay,
    }
})
export default class ValorantMatchScroller extends Vue {
    @Prop({type: Array, default: []})
    matches! : ValorantPlayerMatchSummary[]
}

</script>

<style scoped>

</style>