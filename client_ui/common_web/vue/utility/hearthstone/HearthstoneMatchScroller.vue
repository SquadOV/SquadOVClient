<template>
    <div>
        <hearthstone-match-summary-display
            class="mb-4"
            v-for="(match, index) in matches"
            :match-id="match"
            :key="index"
            :user-id="userId"
        >
        </hearthstone-match-summary-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import HearthstoneMatchSummaryDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchSummaryDisplay.vue'

@Component({
    components: {
        HearthstoneMatchSummaryDisplay
    }
})
export default class HearthstoneMatchScroller extends Vue {
    @Prop({type: Array, default: () => []})
    matches! : string[]

    @Prop({required: true})
    userId!: number
}

</script>

<style scoped>

</style>