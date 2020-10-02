<template>
    <loading-container :is-loading="!allTasks">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <aimlab-task-scroller
                    :tasks="allTasks"
                    v-if="allTasks.length > 0"
                >
                </aimlab-task-scroller>

                <div class="text-center" v-else>
                    <span class="text-h5">No Aim Lab tasks found.</span>
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
import AimlabTaskScroller from '@client/vue/utility/aimlab/AimlabTaskScroller.vue'

@Component({
    components: {
        LoadingContainer,
        AimlabTaskScroller
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