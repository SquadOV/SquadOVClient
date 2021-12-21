<template>
    <recent-recorded-matches
        title="All Games (with VODs)"
        enable-select
        :user-id="userId"
        disable-mini
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
export default class LogGameChooser extends Vue {
    @Prop({required: true})
    userId!: number
}

</script>

<style scoped>

.choice {
    margin-left: 8px;
    margin-right: 8px;
}

</style>