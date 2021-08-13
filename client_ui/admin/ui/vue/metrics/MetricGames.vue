<template>
    <generic-metric-container
        title="Matches (with VODs)"
        :metric="metric"
    >
    </generic-metric-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import GenericMetricContainer from '@client/vue/metrics/GenericMetricContainer.vue'
import { Metrics } from '@client/ts/metrics'

@Component({
    components: {
        GenericMetricContainer,
    }
})
export default class MetricGames extends Vue {
    get metric(): Metrics {
        return Metrics.Games
    }
}

</script>