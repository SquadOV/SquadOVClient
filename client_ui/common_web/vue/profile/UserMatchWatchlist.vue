<template>
    <recent-recorded-matches
        title="Match Watchlist"
        :user-id="userId"
        only-watchlist
    >
    </recent-recorded-matches>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import RecentRecordedMatches from '@client/vue/log/RecentRecordedMatches.vue'

@Component({
    components: {
        RecentRecordedMatches
    }
})
export default class UserMatchWatchlist extends Vue {
    @Prop({required: true})
    userId!: number
}

</script>