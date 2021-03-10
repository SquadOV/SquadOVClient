<template>
    <generic-metric-container
        title="Churn"
    >
    </generic-metric-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import GenericMetricContainer from '@client/vue/metrics/GenericMetricContainer.vue'

@Component({
    components: {
        GenericMetricContainer,
    }
})
export default class MetricChurn extends Vue {
}

</script>