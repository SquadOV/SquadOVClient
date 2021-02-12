<template>
    <div class="d-flex flex-column flex-grow-1 full-width">
        <stat-container
            title="Stat Visualization"
            editable
            :user-id="$store.state.currentUser.id"
        >
            <template v-slot:default="{ data }">
                <line-graph
                    :series-data="data"
                >
                </line-graph>
            </template>
        </stat-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import StatContainer from '@client/vue/stats/StatContainer.vue'
import LineGraph from '@client/vue/stats/LineGraph.vue'

@Component({
    components: {
        StatContainer,
        LineGraph,
    }
})
export default class VizStats extends Vue {
}

</script>

<style scoped>

</style>