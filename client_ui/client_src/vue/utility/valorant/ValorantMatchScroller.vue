<template>
    <div>
        <valorant-player-match-summary-display
            class="mb-4"
            v-for="match in limitedMatches"
            :key="match.matchId"
            :match="match"
        >
        </valorant-player-match-summary-display>

        <v-btn
            v-if="!loadedAllMatches"
            color="primary"
            block
            @click="loadMore"  
        >
            Load More
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantPlayerMatchSummary } from '@client/js/valorant/valorant_matches'
import ValorantPlayerMatchSummaryDisplay from '@client/vue/utility/valorant/ValorantPlayerMatchSummaryDisplay.vue'

const loadMoreMatches: number = 20

@Component({
    components: {
        ValorantPlayerMatchSummaryDisplay,
    }
})
export default class ValorantMatchScroller extends Vue {
    @Prop({type: Array, default: []})
    matches! : ValorantPlayerMatchSummary[]

    maxMatchCount: number = loadMoreMatches

    get loadedAllMatches() : boolean {
        return this.maxMatchCount >= this.matches.length
    }

    get limitedMatches() : ValorantPlayerMatchSummary[] {
        return this.matches.slice(0, this.maxMatchCount)
    }

    loadMore() {
        this.maxMatchCount += loadMoreMatches
    }
}

</script>

<style scoped>

</style>