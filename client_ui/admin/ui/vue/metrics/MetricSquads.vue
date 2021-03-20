<template>
    <generic-metric-container
        title="Squads"
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
export default class MetricSquadInvites extends Vue {
    get metric(): Metrics {
        return Metrics.Squads
    }
}

</script>