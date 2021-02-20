<template>
    <div>
        <valorant-player-match-summary-display
            class="mb-4"
            v-for="match in matches"
            :key="match.matchUuid"
            :match="match"
            :user-id="userId"
            :account="account"
        >
        </valorant-player-match-summary-display>

        <v-btn
            v-if="canLoadMore"
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

@Component({
    components: {
        ValorantPlayerMatchSummaryDisplay,
    }
})
export default class ValorantMatchScroller extends Vue {
    @Prop({type: Array, default: []})
    matches! : ValorantPlayerMatchSummary[]

    @Prop({type: Boolean})
    canLoadMore!: boolean

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    account!: string

    loadMore() {
        this.$emit('load-more')
    }
}

</script>

<style scoped>

</style>