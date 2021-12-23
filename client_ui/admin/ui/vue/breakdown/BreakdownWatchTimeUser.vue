<template>
    <generic-breakdown-container
        title="Watch Time (User)"
        :breakdown="breakdown"
    >
    </generic-breakdown-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import GenericBreakdownContainer from '@client/vue/breakdown/GenericBreakdownContainer.vue'
import { Breakdown } from '@client/ts/breakdown'

@Component({
    components: {
        GenericBreakdownContainer,
        DateRangePicker,
    }
})
export default class BreakdownWatchTimeUser extends Vue {
    get breakdown(): Breakdown {
        return Breakdown.WatchTimeUser
    }
}

</script>