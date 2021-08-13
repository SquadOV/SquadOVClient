<template>
    <generic-metric-container
        title="Clips"
        :metric="metric"
        :extra="extra"
    >
        <v-checkbox
            v-model="useTimeHours"
            dense
            hide-details
            label="Show Time (hours)"
        >
        </v-checkbox>
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
export default class MetricVods extends Vue {
    useTimeHours: boolean = false

    get extra(): any {
        return {
            'useTimeHours': this.useTimeHours.toString(),
        }
    }

    get metric(): Metrics {
        return Metrics.Clips
    }
}

</script>