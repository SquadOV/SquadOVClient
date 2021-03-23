<template>
    <recent-recorded-matches
        title="Favorite Matches"
        :user-id="userId"
        only-favorite
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
export default class UserMatchFavorites extends Vue {
    @Prop({required: true})
    userId!: number
}

</script>