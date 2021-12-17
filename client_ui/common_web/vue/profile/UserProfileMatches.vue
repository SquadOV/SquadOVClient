<template>
    <recent-recorded-matches
        :user-id="userId"
        title=""
        disable-mini
        profile
        :access-token="accessToken"
    >
    </recent-recorded-matches>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import RecentRecordedMatches from '@client/vue/log/RecentRecordedMatches.vue'

@Component({
    components: {
        RecentRecordedMatches
    }
})
export default class UserProfileMatches extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    accessToken!: string
}

</script>