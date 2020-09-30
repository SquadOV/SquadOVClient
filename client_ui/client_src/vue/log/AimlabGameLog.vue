<template>
    <loading-container :is-loading="!allTasks">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <div
                    v-for="(item, idx) in allTasks"
                    class="mb-4"
                    :key="idx"
                >
                    <aimlab-task-summary-display
                        :task="item"
                    >
                    </aimlab-task-summary-display>
                </div>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { apiClient, ApiData } from '@client/js/api'
import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'

@Component({
    components: {
        LoadingContainer,
        AimlabTaskSummaryDisplay
    }
})
export default class AimlabGameLog extends Vue {
    allTasks : AimlabTaskData[] | null = null

    refreshData() {
        apiClient.allAimlabTaskData().then((resp : ApiData<AimlabTaskData[]>) => {
            this.allTasks = resp.data
        }).catch((err : any) => {
            console.log('Failed to list Aim Lab tasks: ' + err);
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

</style>